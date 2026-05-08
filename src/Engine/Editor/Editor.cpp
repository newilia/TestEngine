#include "Engine/Editor/Editor.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/Commands/CutEntityCommand.h"
#include "Engine/Editor/Commands/CutNodeCommand.h"
#include "Engine/Editor/Commands/DeleteEntityCommand.h"
#include "Engine/Editor/Commands/DeleteNodeCommand.h"
#include "Engine/Editor/Commands/PasteEntityCommand.h"
#include "Engine/Editor/Commands/PasteNodeCommand.h"
#include "Engine/Editor/EditorVisualTheme.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <optional>
#include <unordered_set>
#include <vector>

namespace {

	constexpr const char kEditorDockHostWindow[] = "EditorDockHost";
	constexpr const char kEditorDockSpaceId[] = "EditorDockSpace";
	constexpr const char kSceneWindowTitle[] = "Scene";
	constexpr const char kInspectorWindowTitle[] = "Inspector";
	constexpr const char kToolsWindowTitle[] = "Tools";
	constexpr const char kDebugWindowTitle[] = "Debug";

	// Apply a Left | Center | Right split once when the dock root has no saved split and no docked
	// windows yet (first launch or cleared dock state). If ini already restored a split or any
	// docked window, leave layout alone.
	void TryApplyDefaultEditorDockLayout(ImGuiID dockspace_id, const ImVec2& dockspace_size) {
		static bool layout_finished = false;
		if (layout_finished) {
			return;
		}

		ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
		if (node != nullptr && (node->IsSplitNode() || node->Windows.Size > 0)) {
			layout_finished = true;
			return;
		}

		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

		ImGuiID id_left = 0;
		ImGuiID id_center = 0;
		ImGuiID id_right = 0;
		ImGuiID id_left_top = 0;
		ImGuiID id_left_bottom = 0;
		ImGuiID id_right_top = 0;
		ImGuiID id_right_bottom = 0;
		ImGuiID id_main = dockspace_id;

		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Left, 0.25f, &id_left, &id_main);
		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Right, 0.33f, &id_right, &id_center);
		ImGui::DockBuilderSplitNode(id_left, ImGuiDir_Down, 0.55f, &id_left_bottom, &id_left_top);
		ImGui::DockBuilderSplitNode(id_right, ImGuiDir_Down, 0.55f, &id_right_bottom, &id_right_top);

		ImGui::DockBuilderDockWindow(kSceneWindowTitle, id_left_top);
		ImGui::DockBuilderDockWindow(kInspectorWindowTitle, id_left_bottom);
		ImGui::DockBuilderDockWindow(kToolsWindowTitle, id_right_top);
		ImGui::DockBuilderDockWindow(kDebugWindowTitle, id_right_bottom);

		ImGui::DockBuilderFinish(dockspace_id);
		layout_finished = true;
	}

	bool IsNodeInSubtree(const std::shared_ptr<SceneNode>& candidate, const std::shared_ptr<SceneNode>& treeRoot) {
		if (!treeRoot || !candidate) {
			return false;
		}
		for (auto cur = candidate; cur; cur = cur->GetParent()) {
			if (cur == treeRoot) {
				return true;
			}
		}
		return false;
	}

	using Engine::EditorVisualTheme::kHierarchySelectionChildOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionFallbackHalfSize;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlinePadPx;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineThickness;

	std::optional<sf::FloatRect> TryGetHierarchySelectionBounds(const SceneNode& node) {
		sf::Transform fullTransform = node.GetWorldTransform();
		if (auto visual = node.GetVisual()) {
			const auto bounds = visual->GetLocalBounds();
			if (const auto transform = visual->GetTransform()) {
				fullTransform *= *transform;
			}
			return Utils::AxisAlignedBoundsAfterTransform(fullTransform, bounds);
		}
		return std::nullopt;
	}

	void AppendHierarchyAabbOutlineLines(
	    sf::VertexArray& lines, const sf::FloatRect& bounds, const sf::Color& outlineColor, float padPx) {
		const float x0 = bounds.position.x - padPx;
		const float y0 = bounds.position.y - padPx;
		const float x1 = bounds.position.x + bounds.size.x + padPx;
		const float y1 = bounds.position.y + bounds.size.y + padPx;

		const std::size_t startIndex = lines.getVertexCount();
		lines.resize(startIndex + 8);
		lines[startIndex].position = {x0, y0};
		lines[startIndex].color = outlineColor;
		lines[startIndex + 1].position = {x1, y0};
		lines[startIndex + 1].color = outlineColor;
		lines[startIndex + 2].position = {x1, y0};
		lines[startIndex + 2].color = outlineColor;
		lines[startIndex + 3].position = {x1, y1};
		lines[startIndex + 3].color = outlineColor;
		lines[startIndex + 4].position = {x1, y1};
		lines[startIndex + 4].color = outlineColor;
		lines[startIndex + 5].position = {x0, y1};
		lines[startIndex + 5].color = outlineColor;
		lines[startIndex + 6].position = {x0, y1};
		lines[startIndex + 6].color = outlineColor;
		lines[startIndex + 7].position = {x0, y0};
		lines[startIndex + 7].color = outlineColor;
	}

	void CollectHierarchyFallbackMarker(
	    const SceneNode& node, const sf::Color& outlineColor, std::vector<sf::CircleShape>& outCircles) {
		const sf::Vector2f pos = Utils::GetWorldPos(node.shared_from_this());
		sf::CircleShape marker(kHierarchySelectionFallbackHalfSize);
		marker.setOrigin({kHierarchySelectionFallbackHalfSize, kHierarchySelectionFallbackHalfSize});
		marker.setPosition(pos);
		marker.setFillColor(sf::Color::Transparent);
		marker.setOutlineColor(outlineColor);
		marker.setOutlineThickness(kHierarchySelectionOutlineThickness);
		outCircles.push_back(std::move(marker));
	}

	void GatherDescendantHierarchySelectionOutlines(const SceneNode& parent,
	    const std::unordered_set<const SceneNode*>& selectedSet, sf::VertexArray& lineOutlines,
	    std::vector<sf::CircleShape>& fallbackMarkers) {
		for (const auto& child : parent.GetChildren()) {
			if (!child || !child->IsEnabled() || !child->IsVisible()) {
				continue;
			}
			const auto* childPtr = static_cast<const SceneNode*>(child.get());
			if (!selectedSet.contains(childPtr)) {
				if (const std::optional<sf::FloatRect> bb = TryGetHierarchySelectionBounds(*child)) {
					const sf::FloatRect& b = *bb;
					if (b.size.x > 0.f && b.size.y > 0.f) {
						AppendHierarchyAabbOutlineLines(
						    lineOutlines, b, kHierarchySelectionChildOutlineColor, kHierarchySelectionOutlinePadPx);
					}
					else {
						CollectHierarchyFallbackMarker(*child, kHierarchySelectionChildOutlineColor, fallbackMarkers);
					}
				}
				else {
					CollectHierarchyFallbackMarker(*child, kHierarchySelectionChildOutlineColor, fallbackMarkers);
				}
			}
			GatherDescendantHierarchySelectionOutlines(*child, selectedSet, lineOutlines, fallbackMarkers);
		}
	}

	void GatherPrimaryHierarchySelectionOutline(
	    const SceneNode& node, sf::VertexArray& lineOutlines, std::vector<sf::CircleShape>& fallbackMarkers) {
		if (const std::optional<sf::FloatRect> bb = TryGetHierarchySelectionBounds(node)) {
			const sf::FloatRect& b = *bb;
			if (b.size.x > 0.f && b.size.y > 0.f) {
				AppendHierarchyAabbOutlineLines(
				    lineOutlines, b, kHierarchySelectionOutlineColor, kHierarchySelectionOutlinePadPx);
				return;
			}
		}
		CollectHierarchyFallbackMarker(node, kHierarchySelectionOutlineColor, fallbackMarkers);
	}
} // namespace

namespace Engine {
	void Editor::Toggle() {
		_isOpen = !_isOpen;
	}

	void Editor::SetIsOpen(bool isOpen) {
		_isOpen = isOpen;
	}

	bool Editor::IsOpen() const {
		return _isOpen;
	}

	void Editor::Update(float /*dt*/) {}

	std::shared_ptr<SceneNode> Editor::GetSelectedNode() const {
		return _sceneHierarchyWidget.GetSelectedNode();
	}

	std::vector<std::shared_ptr<SceneNode>> Editor::GetSelectedNodes() const {
		return _sceneHierarchyWidget.GetSelectedNodes();
	}

	bool Editor::IsNodeSelected(const SceneNode& node) const {
		return _sceneHierarchyWidget.IsNodeSelected(node);
	}

	void Editor::ClearNodeSelection() {
		_sceneHierarchyWidget.ClearSelection();
	}

	void Editor::SetSelectedNode(std::shared_ptr<SceneNode> node) {
		_sceneHierarchyWidget.Select(std::move(node));
	}

	void Editor::ToggleSelectedNode(std::shared_ptr<SceneNode> node) {
		_sceneHierarchyWidget.ToggleSelection(std::move(node));
	}

	void Editor::AddSelectedNodes(const std::vector<std::shared_ptr<SceneNode>>& nodes) {
		for (const auto& node : nodes) {
			_sceneHierarchyWidget.AddToSelection(node);
		}
	}

	void Editor::SetSelectedNodes(std::vector<std::shared_ptr<SceneNode>> nodes) {
		_sceneHierarchyWidget.SetSelection(std::move(nodes));
	}

	bool Editor::CopySelectedNode() {
		return CopyNode(GetSelectedNode());
	}

	bool Editor::CutSelectedNode() {
		return CutNode(GetSelectedNode());
	}

	bool Editor::PasteClipboard() {
		return PasteClipboardOnto(GetSelectedNode());
	}

	bool Editor::CopyNode(const std::shared_ptr<SceneNode>& node) {
		return node && _clipboard.CopyNode(node);
	}

	bool Editor::CutNode(const std::shared_ptr<SceneNode>& node) {
		if (!node || !node->GetParent()) {
			return false;
		}
		if (!_clipboard.CopyNode(node)) {
			return false;
		}
		const auto cutRoot = node;
		const bool executed = _history.Execute(std::make_unique<EditorCommands::CutNodeCommand>(node));
		if (executed && IsNodeInSubtree(GetSelectedNode(), cutRoot)) {
			ClearNodeSelection();
		}
		return executed;
	}

	bool Editor::PasteClipboardOnto(const std::shared_ptr<SceneNode>& parent) {
		if (!parent) {
			return false;
		}
		if (_clipboard.HasNode()) {
			auto pasted = _clipboard.InstantiateNode();
			if (!pasted) {
				return false;
			}
			const bool executed = _history.Execute(std::make_unique<EditorCommands::PasteNodeCommand>(parent, pasted));
			if (executed) {
				SetSelectedNode(std::move(pasted));
			}
			return executed;
		}
		if (_clipboard.HasEntity()) {
			auto entity = _clipboard.InstantiateEntity();
			if (!entity) {
				return false;
			}
			return _history.Execute(
			    std::make_unique<EditorCommands::PasteEntityCommand>(parent, entity, _clipboard.GetEntitySlot()));
		}
		return false;
	}

	bool Editor::CanPasteClipboard() const {
		return _clipboard.HasNode() || _clipboard.HasEntity();
	}

	bool Editor::DeleteSelectedNode() {
		return DeleteNode(GetSelectedNode());
	}

	bool Editor::DeleteNode(const std::shared_ptr<SceneNode>& node) {
		if (!node || !node->GetParent()) {
			return false;
		}
		const auto removedRoot = node;
		const bool executed = _history.Execute(std::make_unique<EditorCommands::DeleteNodeCommand>(node));
		if (executed && IsNodeInSubtree(GetSelectedNode(), removedRoot)) {
			ClearNodeSelection();
		}
		return executed;
	}

	bool Editor::DeleteEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot) {
		if (!entity || slot == EntitySlot::Transform) {
			return false;
		}
		auto node = GetSelectedNode();
		if (!node) {
			return false;
		}
		return _history.Execute(std::make_unique<EditorCommands::DeleteEntityCommand>(node, entity, slot));
	}

	bool Editor::CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot) {
		return entity && _clipboard.CopyEntity(entity, slot);
	}

	bool Editor::CutEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot) {
		if (!entity || slot == EntitySlot::Transform) {
			return false;
		}
		auto node = GetSelectedNode();
		if (!node) {
			return false;
		}
		if (!_clipboard.CopyEntity(entity, slot)) {
			return false;
		}
		return _history.Execute(std::make_unique<EditorCommands::CutEntityCommand>(node, entity, slot));
	}

	bool Editor::CanPasteEntityToSelectedNode() const {
		return GetSelectedNode() && _clipboard.HasEntity();
	}

	EditorToolManager& Editor::GetEditorToolManager() {
		return *_editorToolManager;
	}

	const EditorToolManager& Editor::GetEditorToolManager() const {
		return *_editorToolManager;
	}

	void Editor::DrawViewportSelectionOverlay(sf::RenderWindow& window) {
		if (!_isOpen) {
			return;
		}
		const auto selectedNodes = GetSelectedNodes();
		if (selectedNodes.empty()) {
			return;
		}

		std::unordered_set<const SceneNode*> selectionSet;
		selectionSet.reserve(selectedNodes.size());
		for (const auto& selected : selectedNodes) {
			if (!selected || !selected->IsEnabled() || !selected->IsVisible()) {
				continue;
			}
			selectionSet.insert(static_cast<const SceneNode*>(selected.get()));
		}

		sf::VertexArray descendantLines(sf::PrimitiveType::Lines);
		sf::VertexArray primaryLines(sf::PrimitiveType::Lines);
		std::vector<sf::CircleShape> descendantFallbackMarkers;
		std::vector<sf::CircleShape> primaryFallbackMarkers;

		for (const auto& selected : selectedNodes) {
			if (!selected || !selected->IsEnabled() || !selected->IsVisible()) {
				continue;
			}
			GatherDescendantHierarchySelectionOutlines(
			    *selected, selectionSet, descendantLines, descendantFallbackMarkers);
			GatherPrimaryHierarchySelectionOutline(*selected, primaryLines, primaryFallbackMarkers);
		}

		sf::RenderStates worldOnly{};
		if (descendantLines.getVertexCount() > 0) {
			window.draw(descendantLines, worldOnly);
		}
		for (const auto& marker : descendantFallbackMarkers) {
			window.draw(marker, worldOnly);
		}

		if (primaryLines.getVertexCount() > 0) {
			window.draw(primaryLines, worldOnly);
		}
		for (const auto& marker : primaryFallbackMarkers) {
			window.draw(marker, worldOnly);
		}
	}

	void Editor::Draw() {
		if (!_isOpen) {
			return;
		}

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		                              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
		                              ImGuiWindowFlags_NoNavFocus;
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
			host_flags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin(kEditorDockHostWindow, nullptr, host_flags);
		ImGui::PopStyleVar(3);

		const ImGuiID dockspace_id = ImGui::GetID(kEditorDockSpaceId);
		TryApplyDefaultEditorDockLayout(dockspace_id, ImGui::GetWindowSize());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();

		if (ImGui::Begin(kSceneWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_sceneHierarchyWidget.Draw(Engine::MainContext::GetInstance().GetScene());
		}
		ImGui::End();

		if (ImGui::Begin(kToolsWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_editorToolsWidget.Draw(GetEditorToolManager());
		}
		ImGui::End();

		if (ImGui::Begin(kInspectorWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_nodeInspectorWidget.Draw(GetSelectedNodes());
		}
		ImGui::End();

		if (ImGui::Begin(kDebugWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None)) {
				if (ImGui::BeginTabItem("Settings")) {
					_debugSettingsWidget.Draw();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Style")) {
					ImGui::ShowStyleEditor();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void Editor::OnEvent(const sf::Event& event) {
		/* TODO event.visit() ? */
		if (const auto* e = event.getIf<sf::Event::Resized>()) {
			OnResize(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
			OnKeyPress(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::KeyReleased>()) {
			OnKeyRelease(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
			OnMouseMove(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
			OnMouseButtonPressed(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::MouseButtonReleased>()) {
			OnMouseButtonReleased(*e);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::MouseWheelScrolled>()) {
			OnMouseWheelScrolled(*e);
			return;
		}
	}

	void Editor::OnResize(const sf::Event::Resized& /*e*/) {}

	void Editor::OnKeyPress(const sf::Event::KeyPressed& e) {
		if (e.code == sf::Keyboard::Key::F1) {
			Toggle();
			return;
		}
		if (!ImGui::GetIO().WantCaptureKeyboard) {
			if (e.control && !e.shift && e.code == sf::Keyboard::Key::Z) {
				(void)_history.Undo();
				return;
			}
			if ((e.control && e.code == sf::Keyboard::Key::Y) ||
			    (e.control && e.shift && e.code == sf::Keyboard::Key::Z)) {
				(void)_history.Redo();
				return;
			}
			if (e.control && e.code == sf::Keyboard::Key::C) {
				(void)CopySelectedNode();
				return;
			}
			if (e.control && e.code == sf::Keyboard::Key::X) {
				(void)CutSelectedNode();
				return;
			}
			if (e.control && e.code == sf::Keyboard::Key::V) {
				(void)PasteClipboard();
				return;
			}
			if (e.code == sf::Keyboard::Key::Delete) {
				(void)DeleteSelectedNode();
				return;
			}
		}
		if (e.control && e.code == sf::Keyboard::Key::D) {
			ClearNodeSelection();
		}
		if (e.code == sf::Keyboard::Key::Space) {
			MainContext::GetInstance().ToggleSimPaused();
		}
		if (_isOpen && !ImGui::GetIO().WantCaptureKeyboard &&
		    GetEditorToolManager().TryActivateToolViaDigitKey(e.code)) {
			return;
		}
	}

	void Editor::OnKeyRelease(const sf::Event::KeyReleased& /*e*/) {}

	void Editor::OnMouseMove(const sf::Event::MouseMoved& e) {
		if (_cameraMoveMouseOriginPos) {
			const sf::Vector2i delta = e.position - *_cameraMoveMouseOriginPos;
			MainContext::GetInstance().MoveCamera(-delta);
			_cameraMoveMouseOriginPos = e.position;
		}
	}

	void Editor::OnMouseButtonPressed(const sf::Event::MouseButtonPressed& e) {
		if (e.button == sf::Mouse::Button::Middle || e.button == sf::Mouse::Button::Right) {
			_cameraMoveMouseOriginPos = e.position;
		}
	}

	void Editor::OnMouseButtonReleased(const sf::Event::MouseButtonReleased& e) {
		if (e.button == sf::Mouse::Button::Middle || e.button == sf::Mouse::Button::Right) {
			_cameraMoveMouseOriginPos.reset();
		}
	}

	void Editor::OnMouseWheelScrolled(const sf::Event::MouseWheelScrolled& e) {
		if (e.wheel == sf::Mouse::Wheel::Vertical) {
			if (!ImGui::GetIO().WantCaptureMouse) {
				auto zoomFactor = 1 - e.delta * 0.15f;
				MainContext::GetInstance().ZoomCamera(zoomFactor, e.position);
			}
		}
	}
} // namespace Engine
