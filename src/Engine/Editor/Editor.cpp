#include "Engine/Editor/Editor.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/ContentPaths.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Commands/AddCircleShapeNodeCommand.h"
#include "Engine/Editor/Commands/AddEmptyNodeCommand.h"
#include "Engine/Editor/Commands/AddPolygonShapeNodeCommand.h"
#include "Engine/Editor/Commands/AddRectangleShapeNodeCommand.h"
#include "Engine/Editor/Commands/AddSceneEntityBatchCommand.h"
#include "Engine/Editor/Commands/CutEntityCommand.h"
#include "Engine/Editor/Commands/CutNodeCommand.h"
#include "Engine/Editor/Commands/DeleteEntityBatchCommand.h"
#include "Engine/Editor/Commands/DeleteEntityCommand.h"
#include "Engine/Editor/Commands/DeleteNodeCommand.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Editor/Commands/MoveNodesInHierarchyCommand.h"
#include "Engine/Editor/Commands/PasteEntityCommand.h"
#include "Engine/Editor/Commands/PasteNodeCommand.h"
#include "Engine/Editor/Commands/SetNodeWorldPositionCommand.h"
#include "Engine/Editor/EditorVisualTheme.h"
#include "Engine/Editor/ImGui/Themes.h"
#include "Engine/Editor/NativeFileDialog.h"
#include "Engine/Serialization/PrefabSerializer.h"
#include "Engine/Serialization/SceneDocumentSerializer.h"
#include "Engine/Serialization/SceneEntityRegistry.h"
#include "Engine/Serialization/SceneSettings/SceneSettingsRegistry.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>
#include <optional>
#include <string>
#include <typeindex>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace {

	constexpr const char kEditorDockHostWindow[] = "EditorDockHost";
	constexpr const char kEditorDockSpaceId[] = "EditorDockSpace";
	constexpr const char kSceneWindowTitle[] = "Scene";
	constexpr const char kInspectorWindowTitle[] = "Inspector";
	constexpr const char kToolsWindowTitle[] = "Tools";
	constexpr const char kDebugWindowTitle[] = "Debug";
	constexpr const char kImGuiStyleTitle[] = "Style";
	constexpr const char kGameBackgroundWindowTitle[] = "Game background";
	using Engine::EditorVisualTheme::kHierarchySelectionChildOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionFallbackHalfSize;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineColor;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlinePadPx;
	using Engine::EditorVisualTheme::kHierarchySelectionOutlineThickness;

	[[nodiscard]] std::optional<Engine::EntitySlot> TrySlotForSceneEntityKind(
	    Engine::Serialization::SceneEntityKind kind) {
		using Engine::EntitySlot;
		using Engine::Serialization::SceneEntityKind;
		switch (kind) {
		case SceneEntityKind::Behaviour:
			return EntitySlot::Behaviour;
		case SceneEntityKind::Visual:
			return EntitySlot::Visual;
		case SceneEntityKind::SortingStrategy:
			return EntitySlot::SortingStrategy;
		case SceneEntityKind::Transform:
			return std::nullopt;
		}
		return std::nullopt;
	}

	[[nodiscard]] std::shared_ptr<EntityOnNode> FindDeletableEntityOnNode(const std::shared_ptr<SceneNode>& node,
	    Engine::EntitySlot slot, const std::optional<std::type_index>& behaviourType) {
		if (!node || slot == Engine::EntitySlot::Transform) {
			return nullptr;
		}
		if (slot == Engine::EntitySlot::Visual) {
			return std::static_pointer_cast<EntityOnNode>(node->GetVisual());
		}
		if (slot == Engine::EntitySlot::SortingStrategy) {
			return std::static_pointer_cast<EntityOnNode>(node->GetSortingStrategy());
		}
		if (slot != Engine::EntitySlot::Behaviour || !behaviourType) {
			return nullptr;
		}
		for (const auto& behaviour : node->GetBehaviours()) {
			if (behaviour && std::type_index(typeid(*behaviour)) == *behaviourType) {
				return behaviour;
			}
		}
		return nullptr;
	}
} // namespace

namespace Engine {

	Editor::Editor() {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		::Editor::Themes::SetupImGuiDraculaStyle();
		ImGuiStyle& style = ImGui::GetStyle();
		style.TreeLinesFlags = ImGuiTreeNodeFlags_DrawLinesFull;
		style.TreeLinesSize = 2.0f;

		MainContext::GetInstance().GetFontManager()->GetDefaultFont();
		ImFontAtlas& atlas = *ImGui::GetIO().Fonts;
		ImFontConfig config;
		config.OversampleH = 2.0f;
		config.OversampleV = 2.0f;
		if (ImFont* font = Verify(atlas.AddFontFromFileTTF("resources/fonts/NotoSans-Medium.ttf", 19.f, &config))) {
			atlas.Build();
			Verify(ImGui::SFML::UpdateFontTexture());
			ImGui::GetIO().FontDefault = font;
		}
	}

	void Editor::Toggle() {
		_isOpen = !_isOpen;
	}

	void Editor::SetIsOpen(bool isOpen) {
		_isOpen = isOpen;
	}

	bool Editor::IsOpen() const {
		return _isOpen;
	}

	void Editor::OnUpdate(const sf::Time dt) {
		if (!_isOpen) {
			return;
		}
		_editorToolManager->OnUpdate(dt);
	}

	void Editor::DrawSceneGrid(sf::RenderWindow& window) {
		if (!_isOpen) {
			return;
		}
		_editorSceneGrid.Draw(window);
	}

	void Editor::Draw(sf::RenderWindow& window) {
		if (!_isOpen) {
			return;
		}
		DrawLayout();
		DrawCursorWorldCoordsOverlay(window);
		DrawViewportSelectionOverlay(window);
		_physicsVisualizer.Draw(window);
		_editorToolManager->DrawOverlay(window);
	}

	void Editor::DrawLayout() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImGuiID dockspaceId = ImGui::GetID(kEditorDockSpaceId);

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N")) {
					(void)NewScene();
				}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {
					(void)LoadScene();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					(void)SaveScene();
				}
				if (ImGui::MenuItem("Save As…", "Ctrl+Shift+S")) {
					(void)SaveSceneAs();
				}
				if (ImGui::MenuItem("Reload from disk", "Ctrl+R", false, _currentScenePath.has_value())) {
					(void)ReloadScene();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Instantiate prefab…")) {
					(void)InstantiatePrefab();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				EditorSceneGrid& grid = _editorSceneGrid;
				ImGui::MenuItem("Show Grid", "Ctrl+G", &grid.VisibleMutable());
				ImGui::MenuItem("Snap to Grid", "Ctrl+Shift+G", &grid.SnapEnabledMutable());
				ImGui::MenuItem("Draw selection", nullptr, &DrawSelectionEnabledMutable());
				ImGui::MenuItem("Draw descendants selection", nullptr, &DrawDescendantsSelectionEnabledMutable());
				ImGui::Separator();
				constexpr float kSliderWidth = 180.f;
				ImGui::SetNextItemWidth(kSliderWidth);
				ImGui::SliderInt(
				    "Grid Size", &grid.SizeMutable(), EditorSceneGrid::kMinSize, EditorSceneGrid::kMaxSize);
				ImGui::SetNextItemWidth(kSliderWidth);
				ImGui::SliderInt(
				    "Grid Basis", &grid.BasisMutable(), EditorSceneGrid::kMinBasis, EditorSceneGrid::kMaxBasis);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::DockSpaceOverViewport(dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
		TryApplyDefaultEditorDockLayout(dockspaceId, ImGui::GetWindowSize());
		DrawSaveDocumentKindModal();

		if (ImGui::Begin(kSceneWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_sceneHierarchyWidget.Draw(Engine::MainContext::GetInstance().GetScene());
		}
		ImGui::End();

		if (ImGui::Begin(kToolsWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_editorToolsWidget.Draw(*_editorToolManager);
		}
		ImGui::End();

		if (ImGui::Begin(kImGuiStyleTitle, nullptr, ImGuiWindowFlags_None)) {
			ImGui::ShowStyleEditor();
		}
		ImGui::End();

		if (ImGui::Begin(kGameBackgroundWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_gameBackgroundWidget.Draw();
		}
		ImGui::End();

		if (ImGui::Begin(kInspectorWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_nodeInspectorWidget.Draw(GetSelectedNodes());
		}
		ImGui::End();

		if (ImGui::Begin(kDebugWindowTitle, nullptr, ImGuiWindowFlags_None)) {
			_debugSettingsWidget.Draw();
		}
		ImGui::End();
	}

	void Editor::TryApplyDefaultEditorDockLayout(ImGuiID dockspaceId, const ImVec2& dockspaceSize) const {
		if (_isLayoutFinished) {
			return;
		}

		ImGuiDockNode* dockspaceNode = ImGui::DockBuilderGetNode(dockspaceId);
		if (dockspaceNode != nullptr && (dockspaceNode->IsSplitNode() || dockspaceNode->Windows.Size > 0)) {
			_isLayoutFinished = true;
			return;
		}

		ImGuiID id_left = 0;
		ImGuiID id_center = 0;
		ImGuiID id_right = 0;
		ImGuiID id_left_top = 0;
		ImGuiID id_left_bottom = 0;
		ImGuiID id_right_top = 0;
		ImGuiID id_right_bottom = 0;
		ImGuiID id_main = dockspaceId;

		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Left, 0.20f, &id_left, &id_main);
		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Right, 0.25f, &id_right, &id_center);
		ImGui::DockBuilderSplitNode(id_left, ImGuiDir_Down, 0.5f, &id_left_bottom, &id_left_top);
		ImGui::DockBuilderSplitNode(id_right, ImGuiDir_Down, 0.8f, &id_right_bottom, &id_right_top);

		ImGui::DockBuilderDockWindow(kSceneWindowTitle, id_left_top);

		ImGui::DockBuilderDockWindow(kInspectorWindowTitle, id_left_bottom);

		ImGui::DockBuilderDockWindow(kToolsWindowTitle, id_right_top);

		ImGui::DockBuilderDockWindow(kImGuiStyleTitle, id_right_bottom);
		ImGui::DockBuilderDockWindow(kGameBackgroundWindowTitle, id_right_bottom);
		ImGui::DockBuilderDockWindow(kDebugWindowTitle, id_right_bottom);

		ImGui::DockBuilderFinish(dockspaceId);
		_isLayoutFinished = true;
	}

	void Editor::DrawCursorWorldCoordsOverlay(sf::RenderWindow& window) {
		const sf::Vector2i pixel = sf::Mouse::getPosition(window);
		const auto windowSize = window.getSize();
		if (pixel.x < 0 || pixel.y < 0 || static_cast<unsigned>(pixel.x) >= windowSize.x ||
		    static_cast<unsigned>(pixel.y) >= windowSize.y) {
			return;
		}

		const sf::Vector2f world = Utils::MapWindowPixelToWorld(window, pixel);
		const ImVec2 anchor(static_cast<float>(pixel.x), static_cast<float>(pixel.y));
		ImGui::SetNextWindowBgAlpha(0.25f);
		ImGui::SetNextWindowPos(ImVec2(anchor.x + 15.f, anchor.y + 12.f), ImGuiCond_Always);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3.f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.92f, 0.5f));

		constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		                                   ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
		                                   ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking |
		                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;

		if (ImGui::Begin("##CursorWorldXY", nullptr, flags)) {
			if (const std::optional<std::string> shapeText = _editorToolManager->TryGetActiveToolCursorOverlayText()) {
				ImGui::TextUnformatted(shapeText->c_str());
			}
			ImGui::Text("%.1f  %.1f", world.x, world.y);
		}
		ImGui::End();

		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(3);
	}

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
			const EntitySlot slot = _clipboard.GetEntitySlot();
			if (slot == EntitySlot::Transform) {
				auto transform = _clipboard.InstantiateTransform();
				if (!transform) {
					return false;
				}
				return _history.Execute(
				    std::make_unique<EditorCommands::PasteEntityCommand>(parent, nullptr, slot, std::move(transform)));
			}
			auto entity = _clipboard.InstantiateEntity();
			if (!entity) {
				return false;
			}
			return _history.Execute(std::make_unique<EditorCommands::PasteEntityCommand>(parent, entity, slot));
		}
		return false;
	}

	bool Editor::CanPasteClipboard() const {
		return _clipboard.HasNode() || _clipboard.HasEntity();
	}

	bool Editor::DeleteSelectedNodes() {
		for (const auto& node : GetSelectedNodes()) {
			DeleteNode(node);
		}
		return true;
	}

	bool Editor::AddEmptyChildNode(const std::shared_ptr<SceneNode>& parent) {
		if (!parent) {
			return false;
		}
		const std::size_t index = parent->GetChildren().size();
		return _history.Execute(std::make_unique<EditorCommands::AddEmptyNodeCommand>(parent, index));
	}

	bool Editor::AddEmptySiblingNode(const std::shared_ptr<SceneNode>& sibling) {
		if (!sibling) {
			return false;
		}
		const auto parent = sibling->GetParent();
		if (!parent) {
			return false;
		}
		const auto& children = parent->GetChildren();
		const auto it = std::find(children.begin(), children.end(), sibling);
		if (it == children.end()) {
			return false;
		}
		const std::size_t index = static_cast<std::size_t>(std::distance(children.begin(), it)) + 1;
		return _history.Execute(std::make_unique<EditorCommands::AddEmptyNodeCommand>(parent, index));
	}

	bool Editor::AddRectangleShape(const std::shared_ptr<SceneNode>& parent, const sf::Vector2f centerWorld,
	    const sf::Vector2f size, const bool attachPhysics, const Utils::HsvShapeColors colors) {
		if (!parent) {
			return false;
		}
		const std::size_t index = parent->GetChildren().size();
		return _history.Execute(std::make_unique<EditorCommands::AddRectangleShapeNodeCommand>(
		    parent, index, centerWorld, size, attachPhysics, colors));
	}

	bool Editor::AddCircleShape(const std::shared_ptr<SceneNode>& parent, const sf::Vector2f centerWorld,
	    const float radius, const bool attachPhysics, const Utils::HsvShapeColors colors) {
		if (!parent) {
			return false;
		}
		const std::size_t index = parent->GetChildren().size();
		return _history.Execute(std::make_unique<EditorCommands::AddCircleShapeNodeCommand>(
		    parent, index, centerWorld, radius, attachPhysics, colors));
	}

	bool Editor::AddPolygonShape(const std::shared_ptr<SceneNode>& parent, const sf::Vector2f centerWorld,
	    std::vector<sf::Vector2f> localPoints, const bool attachPhysics, const Utils::HsvShapeColors colors) {
		if (!parent || localPoints.size() < 3) {
			return false;
		}
		const std::size_t index = parent->GetChildren().size();
		return _history.Execute(std::make_unique<EditorCommands::AddPolygonShapeNodeCommand>(
		    parent, index, centerWorld, std::move(localPoints), attachPhysics, colors));
	}

	bool Editor::CommitNodeWorldPosition(const std::shared_ptr<SceneNode>& node, sf::Vector2f previousWorldPos) {
		if (!node) {
			return false;
		}
		const sf::Vector2f newWorldPos = Utils::GetWorldPos(node);
		const sf::Vector2f delta = newWorldPos - previousWorldPos;
		constexpr float kEpsilon = 0.001f;
		if (delta.x * delta.x + delta.y * delta.y < kEpsilon * kEpsilon) {
			return false;
		}
		return _history.Execute(
		    std::make_unique<EditorCommands::SetNodeWorldPositionCommand>(node, previousWorldPos, newWorldPos));
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

	bool Editor::MoveNodesInHierarchy(const std::vector<std::shared_ptr<SceneNode>>& nodes,
	    const std::shared_ptr<SceneNode>& newParent, std::size_t newIndex) {
		if (!newParent || nodes.empty()) {
			return false;
		}

		std::vector<EditorCommands::MoveNodesInHierarchyCommand::Entry> entries;
		entries.reserve(nodes.size());
		for (const auto& node : nodes) {
			if (!node || !node->GetParent()) {
				return false;
			}
			if (EditorCommands::IsNodeInSubtree(newParent, node)) {
				return false;
			}
			const auto parent = node->GetParent();
			const auto& children = parent->GetChildren();
			const auto it = std::find(children.begin(), children.end(), node);
			if (it == children.end()) {
				return false;
			}
			entries.push_back({node, parent, static_cast<std::size_t>(std::distance(children.begin(), it))});
		}

		bool isNoOp = true;
		for (std::size_t i = 0; i < entries.size(); ++i) {
			const auto oldParent = entries[i].oldParent.lock();
			if (!oldParent || oldParent != newParent || entries[i].oldIndex != newIndex + i) {
				isNoOp = false;
				break;
			}
		}
		if (isNoOp) {
			return false;
		}

		return _history.Execute(
		    std::make_unique<EditorCommands::MoveNodesInHierarchyCommand>(std::move(entries), newParent, newIndex));
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

	bool Editor::DeleteEntitiesFromNodes(const std::vector<std::shared_ptr<SceneNode>>& nodes, EntitySlot slot,
	    std::optional<std::type_index> behaviourType) {
		if (slot == EntitySlot::Transform) {
			return false;
		}
		if (slot == EntitySlot::Behaviour && !behaviourType) {
			return false;
		}
		std::vector<EditorCommands::DeleteEntityBatchCommand::Entry> entries;
		entries.reserve(nodes.size());
		for (const auto& node : nodes) {
			if (!node) {
				continue;
			}
			const auto entity = FindDeletableEntityOnNode(node, slot, behaviourType);
			if (!entity) {
				return false;
			}
			EditorCommands::DeleteEntityBatchCommand::Entry entry;
			entry.node = node;
			entry.entity = entity;
			entry.slot = slot;
			entries.push_back(std::move(entry));
		}
		if (entries.empty()) {
			return false;
		}
		return _history.Execute(std::make_unique<EditorCommands::DeleteEntityBatchCommand>(std::move(entries)));
	}

	bool Editor::CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot) {
		return entity && _clipboard.CopyEntity(entity, slot);
	}

	bool Editor::CopyNodeTransform(const std::shared_ptr<SceneNode>& node) {
		return node && _clipboard.CopyNodeTransform(node);
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

	bool Editor::AddSceneEntityFromRegistry(
	    const std::vector<std::shared_ptr<SceneNode>>& nodes, std::string_view typeId) {
		std::vector<std::shared_ptr<SceneNode>> targets;
		targets.reserve(nodes.size());
		for (const auto& node : nodes) {
			if (node) {
				targets.push_back(node);
			}
		}
		if (targets.empty()) {
			return false;
		}
		const Serialization::SceneEntityRegistry& registry = Serialization::SceneEntityRegistry::GetInstance();
		const Serialization::SceneEntityRegistration* registration = nullptr;
		for (const Serialization::SceneEntityRegistration& r : registry.GetAll()) {
			if (r.typeId == typeId) {
				registration = &r;
				break;
			}
		}
		if (!registration) {
			return false;
		}
		const Serialization::SceneEntityKind kind = registration->kind;
		if (kind == Serialization::SceneEntityKind::Transform) {
			return false;
		}
		const std::optional<EntitySlot> slot = TrySlotForSceneEntityKind(kind);
		if (!slot) {
			return false;
		}
		if (kind == Serialization::SceneEntityKind::Visual) {
			for (const auto& node : targets) {
				if (node->GetVisual()) {
					return false;
				}
			}
		}
		if (kind == Serialization::SceneEntityKind::SortingStrategy) {
			for (const auto& node : targets) {
				if (node->GetSortingStrategy()) {
					return false;
				}
			}
		}
		std::vector<EditorCommands::AddSceneEntityBatchCommand::Entry> entries;
		entries.reserve(targets.size());
		for (const auto& node : targets) {
			std::shared_ptr<EntityOnNode> entity = registry.CreateByTypeId(typeId);
			if (!entity) {
				return false;
			}
			if (*slot == EntitySlot::Behaviour) {
				if (!std::dynamic_pointer_cast<Behaviour>(entity)) {
					return false;
				}
			}
			else if (*slot == EntitySlot::Visual) {
				if (!std::dynamic_pointer_cast<Visual>(entity)) {
					return false;
				}
			}
			else if (*slot == EntitySlot::SortingStrategy) {
				if (!std::dynamic_pointer_cast<RelativeSortingStrategy>(entity)) {
					return false;
				}
			}
			EditorCommands::AddSceneEntityBatchCommand::Entry entry;
			entry.node = node;
			entry.entity = std::move(entity);
			entry.slot = *slot;
			entries.push_back(std::move(entry));
		}
		return _history.Execute(std::make_unique<EditorCommands::AddSceneEntityBatchCommand>(std::move(entries)));
	}

	EditorToolManager& Editor::GetEditorToolManager() {
		return *_editorToolManager;
	}

	const EditorToolManager& Editor::GetEditorToolManager() const {
		return *_editorToolManager;
	}

	PhysicsVisualizer& Editor::GetPhysicsVisualizer() {
		return _physicsVisualizer;
	}

	const PhysicsVisualizer& Editor::GetPhysicsVisualizer() const {
		return _physicsVisualizer;
	}

	EditorSceneGrid& Editor::GetEditorSceneGrid() {
		return _editorSceneGrid;
	}

	const EditorSceneGrid& Editor::GetEditorSceneGrid() const {
		return _editorSceneGrid;
	}

	bool Editor::IsDrawSelectionEnabled() const {
		return _isDrawSelectionEnabled;
	}

	bool& Editor::DrawSelectionEnabledMutable() {
		return _isDrawSelectionEnabled;
	}

	bool Editor::IsDrawDescendantsSelectionEnabled() const {
		return _isDrawDescendantsSelectionEnabled;
	}

	bool& Editor::DrawDescendantsSelectionEnabledMutable() {
		return _isDrawDescendantsSelectionEnabled;
	}

	void Editor::DrawViewportSelectionOverlay(sf::RenderWindow& window) {
		if (!_isOpen || (!IsDrawSelectionEnabled() && !IsDrawDescendantsSelectionEnabled())) {
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
			if (IsDrawDescendantsSelectionEnabled()) {
				GatherDescendantHierarchySelectionOutlines(
				    *selected, selectionSet, descendantLines, descendantFallbackMarkers);
			}
			if (IsDrawSelectionEnabled()) {
				GatherPrimaryHierarchySelectionOutline(*selected, primaryLines, primaryFallbackMarkers);
			}
		}

		sf::RenderStates worldOnly{};
		if (IsDrawDescendantsSelectionEnabled()) {
			if (descendantLines.getVertexCount() > 0) {
				window.draw(descendantLines, worldOnly);
			}
			for (const auto& marker : descendantFallbackMarkers) {
				window.draw(marker, worldOnly);
			}
		}

		if (IsDrawSelectionEnabled()) {
			if (primaryLines.getVertexCount() > 0) {
				window.draw(primaryLines, worldOnly);
			}
			for (const auto& marker : primaryFallbackMarkers) {
				window.draw(marker, worldOnly);
			}
		}
	}

	void Editor::OnEvent(const sf::Event& event) {
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
			if (e.control && !e.shift && e.code == sf::Keyboard::Key::S) {
				(void)SaveScene();
				return;
			}
			if (e.control && e.shift && e.code == sf::Keyboard::Key::S) {
				(void)SaveSceneAs();
				return;
			}
			if (e.control && !e.shift && e.code == sf::Keyboard::Key::N) {
				(void)NewScene();
				return;
			}
			if (e.control && !e.shift && e.code == sf::Keyboard::Key::O) {
				(void)LoadScene();
				return;
			}
			if (e.control && !e.shift && e.code == sf::Keyboard::Key::R) {
				(void)ReloadScene();
				return;
			}
			if (_isOpen && e.control && !e.shift && e.code == sf::Keyboard::Key::G) {
				_editorSceneGrid.VisibleMutable() = !_editorSceneGrid.IsVisible();
				return;
			}
			if (_isOpen && e.control && e.shift && e.code == sf::Keyboard::Key::G) {
				_editorSceneGrid.SnapEnabledMutable() = !_editorSceneGrid.IsSnapEnabled();
				return;
			}
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
				(void)DeleteSelectedNodes();
				return;
			}
		}
		if (e.control && e.code == sf::Keyboard::Key::D) {
			ClearNodeSelection();
		}
		if (e.code == sf::Keyboard::Key::Space) {
			MainContext::GetInstance().ToggleSimPaused();
		}
		if (_isOpen && !ImGui::GetIO().WantCaptureKeyboard && _editorToolManager->TryActivateToolViaDigitKey(e.code)) {
			return;
		}
	}

	const char* Editor::DocumentKindLabel() const {
		return _documentKind == Serialization::SceneDocumentKind::Prefab ? "Prefab" : "Scene";
	}

	void Editor::SetCurrentDocument(std::optional<std::filesystem::path> path, Serialization::SceneDocumentKind kind) {
		_documentKind = kind;
		_documentKindChosen = true;
		if (path) {
			_currentScenePath = ResolveContentPath(*path);
		}
		else {
			_currentScenePath.reset();
		}
		MainContext::GetInstance().UpdateMainWindowTitle(_currentScenePath, _documentKind);
	}

	EditorDialogs::SceneFileDialogOptions Editor::MakeSceneFileDialogOptions(
	    Serialization::SceneDocumentKind kind) const {
		EditorDialogs::SceneFileDialogOptions opts;
#ifdef _WIN32
		if (const sf::RenderWindow* window = MainContext::GetInstance().GetMainWindow()) {
			opts.parentHwnd = static_cast<HWND>(window->getNativeHandle());
		}
#endif
		if (_currentScenePath && kind == _documentKind) {
			opts.suggestedFileName = *_currentScenePath;
			opts.initialDirectory = _currentScenePath->parent_path();
		}
		else if (kind == Serialization::SceneDocumentKind::Prefab) {
			opts.initialDirectory = DefaultScenePrefabsDirectory();
			opts.suggestedFileName = DefaultScenePrefabAbsolutePath();
		}
		else {
			opts.initialDirectory = DefaultScenesDirectory();
			opts.suggestedFileName = DefaultSceneAbsolutePath();
		}
		return opts;
	}

	void Editor::DrawSaveDocumentKindModal() {
		if (!_showSaveDocumentKindModal) {
			return;
		}
		if (!ImGui::IsPopupOpen("Save document kind", ImGuiPopupFlags_None)) {
			ImGui::OpenPopup("Save document kind");
		}
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(
		        "Save document kind", &_showSaveDocumentKindModal, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextUnformatted("Choose how to save this document:");
			ImGui::RadioButton("Scene (hierarchy + world settings)", &_saveKindModalSelection, 0);
			ImGui::RadioButton("Prefab (hierarchy only)", &_saveKindModalSelection, 1);
			const bool confirm = ImGui::Button("OK", ImVec2(120.f, 0.f)) ||
			                     (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			                         ImGui::IsKeyPressed(ImGuiKey_Enter, false));
			if (confirm) {
				_documentKind = _saveKindModalSelection == 1 ? Serialization::SceneDocumentKind::Prefab
				                                             : Serialization::SceneDocumentKind::Scene;
				_documentKindChosen = true;
				_showSaveDocumentKindModal = false;
				ImGui::CloseCurrentPopup();
				if (_pendingSaveAs || !_currentScenePath) {
					(void)SaveSceneAs();
				}
				else {
					(void)SaveScene();
				}
				_pendingSaveAs = false;
			}
			ImGui::SameLine();
			const bool cancel = ImGui::Button("Cancel", ImVec2(120.f, 0.f)) ||
			                    (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			                        ImGui::IsKeyPressed(ImGuiKey_Escape, false));
			if (cancel) {
				_showSaveDocumentKindModal = false;
				_pendingSaveAs = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	bool Editor::BeginSaveFlow(bool saveAs) {
		if (_documentKindChosen && (_currentScenePath || saveAs)) {
			return true;
		}
		if (!_documentKindChosen) {
			_pendingSaveAs = saveAs;
			_saveKindModalSelection = _documentKind == Serialization::SceneDocumentKind::Prefab ? 1 : 0;
			_showSaveDocumentKindModal = true;
			return false;
		}
		return true;
	}

	void Editor::ShowSerializationErrorDialog(
	    std::string_view title, const Serialization::SerializationResult& result) const {
		std::string body;
		for (const Serialization::SerializationDiagnostic& d : result.diagnostics) {
			const char* level = d.level == Serialization::SerializationDiagnosticLevel::Warning ? "Warning" : "Error";
			if (!body.empty()) {
				body += "\n\n";
			}
			if (d.path.empty()) {
				body += fmt::format("{}: {}", level, d.message);
			}
			else {
				body += fmt::format("{} ({}): {}", level, d.path, d.message);
			}
		}
		if (body.empty()) {
			body = "Unknown serialization error";
		}
#ifdef _WIN32
		const std::wstring wideTitle = Utils::Utf8ToWide(title);
		const std::wstring wideBody = Utils::Utf8ToWide(body);
		HWND parent = nullptr;
		if (const sf::RenderWindow* window = MainContext::GetInstance().GetMainWindow()) {
			parent = static_cast<HWND>(window->getNativeHandle());
		}
		(void)MessageBoxW(parent, wideBody.c_str(), wideTitle.c_str(), MB_OK | MB_ICONERROR);
#else
		(void)title;
		(void)result;
#endif
	}

	bool Editor::SaveScene() {
		if (!BeginSaveFlow(false)) {
			return false;
		}
		if (!_currentScenePath) {
			return SaveSceneAs();
		}
		const auto scene = MainContext::GetInstance().GetScene();
		if (!scene) {
			return false;
		}
		const Serialization::SerializationResult saveResult =
		    _documentKind == Serialization::SceneDocumentKind::Prefab
		        ? Serialization::SceneDocumentSerializer::SavePrefabDocument(*scene->GetRoot(), *_currentScenePath)
		        : Serialization::SceneDocumentSerializer::SaveSceneDocument(*scene, *_currentScenePath);
		if (!saveResult.isSuccess) {
			ShowSerializationErrorDialog(fmt::format("Save {}", DocumentKindLabel()), saveResult);
			return false;
		}
		return true;
	}

	bool Editor::SaveSceneAs() {
		if (!BeginSaveFlow(true)) {
			return false;
		}
		const auto scene = MainContext::GetInstance().GetScene();
		if (!scene) {
			return false;
		}
		const auto path = EditorDialogs::PickSceneXmlSave(MakeSceneFileDialogOptions(_documentKind));
		if (!path) {
			return false;
		}
		const Serialization::SerializationResult saveResult =
		    _documentKind == Serialization::SceneDocumentKind::Prefab
		        ? Serialization::SceneDocumentSerializer::SavePrefabDocument(*scene->GetRoot(), *path)
		        : Serialization::SceneDocumentSerializer::SaveSceneDocument(*scene, *path);
		if (!saveResult.isSuccess) {
			ShowSerializationErrorDialog(fmt::format("Save {}", DocumentKindLabel()), saveResult);
			return false;
		}
		SetCurrentDocument(*path, _documentKind);
		return true;
	}

	bool Editor::SaveNodeAsPrefab(const std::shared_ptr<SceneNode>& node) {
		if (!node) {
			return false;
		}
		EditorDialogs::SceneFileDialogOptions opts =
		    MakeSceneFileDialogOptions(Serialization::SceneDocumentKind::Prefab);
		const auto path = EditorDialogs::PickSceneXmlSave(opts);
		if (!path) {
			return false;
		}
		const Serialization::SerializationResult saveResult =
		    Serialization::SceneDocumentSerializer::SavePrefabDocument(*node, *path);
		if (!saveResult.isSuccess) {
			ShowSerializationErrorDialog("Save prefab", saveResult);
			return false;
		}
		return true;
	}

	bool Editor::InstantiatePrefab() {
		const auto path =
		    EditorDialogs::PickSceneXmlOpen(MakeSceneFileDialogOptions(Serialization::SceneDocumentKind::Prefab));
		if (!path) {
			return false;
		}
		return InstantiatePrefab(*path);
	}

	bool Editor::InstantiatePrefab(const std::filesystem::path& path) {
		const Serialization::PrefabInstantiateResult loaded =
		    Serialization::PrefabSerializer::InstantiateFromFile(path);
		if (!loaded.result.isSuccess || !loaded.instance) {
			ShowSerializationErrorDialog("Instantiate prefab", loaded.result);
			return false;
		}
		std::shared_ptr<SceneNode> parent = GetSelectedNode();
		if (!parent) {
			if (const auto scene = MainContext::GetInstance().GetScene()) {
				parent = scene->GetRoot();
			}
		}
		if (!parent) {
			return false;
		}
		const auto instance = loaded.instance;
		const bool executed = _history.Execute(std::make_unique<EditorCommands::PasteNodeCommand>(parent, instance));
		if (executed) {
			SetSelectedNode(instance);
		}
		return executed;
	}

	bool Editor::NewScene() {
		Serialization::SceneSettingsRegistry::GetInstance().ApplyAllDefaults();
		const auto scene = std::make_shared<Scene>();
		MainContext::GetInstance().SetScene(scene);
		ClearNodeSelection();
		_history = EditorHistory{};
		SetCurrentDocument(std::nullopt, Serialization::SceneDocumentKind::Scene);
		return true;
	}

	bool Editor::LoadScene() {
		const auto path = EditorDialogs::PickSceneXmlOpen(
		    MakeSceneFileDialogOptions(_documentKindChosen ? _documentKind : Serialization::SceneDocumentKind::Scene));
		if (!path) {
			return false;
		}
		return LoadScene(*path);
	}

	bool Editor::LoadScene(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			return false;
		}
		Serialization::SceneDocumentLoadResult loaded =
		    Serialization::SceneDocumentSerializer::LoadDocumentFromFile(path);
		if (!loaded.result.isSuccess || !loaded.scene) {
			ShowSerializationErrorDialog("Load document", loaded.result);
			return false;
		}
		MainContext::GetInstance().SetScene(loaded.scene);
		ClearNodeSelection();
		_history = EditorHistory{};
		SetCurrentDocument(path, loaded.kind);
		return true;
	}

	bool Editor::ReloadScene() {
		if (!_currentScenePath) {
			return false;
		}
		return LoadScene(*_currentScenePath);
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
				auto& mainContext = MainContext::GetInstance();
				if (auto* window = mainContext.GetMainWindow()) {
					const auto zoomFactor = 1.f - e.delta * 0.15f;
					mainContext.ZoomCamera(zoomFactor, sf::Mouse::getPosition(*window));
				}
			}
		}
	}

	bool Editor::IsNodeInSubtree(
	    const std::shared_ptr<SceneNode>& candidate, const std::shared_ptr<SceneNode>& treeRoot) {
		return EditorCommands::IsNodeInSubtree(candidate, treeRoot);
	}

	std::optional<sf::FloatRect> Editor::TryGetHierarchySelectionBounds(const SceneNode& node) {
		sf::Transform fullTransform = node.GetWorldTransform();
		if (auto visual = node.GetVisual()) {
			const auto bounds = visual->GetLocalBounds();
			if (const auto transform = visual->GetTransform()) {
				fullTransform *= *transform;
			}
			return fullTransform.transformRect(bounds);
		}
		return std::nullopt;
	}

	void Editor::AppendHierarchyAabbOutlineLines(
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

	void Editor::CollectHierarchyFallbackMarker(
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

	void Editor::GatherDescendantHierarchySelectionOutlines(const SceneNode& parent,
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

	void Editor::GatherPrimaryHierarchySelectionOutline(
	    const SceneNode& node, sf::VertexArray& lineOutlines, std::vector<sf::CircleShape>& fallbackMarkers) {
		if (const std::optional<sf::FloatRect> bb = TryGetHierarchySelectionBounds(node)) {
			const sf::FloatRect& b = *bb;
			if (b.size.x > 0.f && b.size.y > 0.f) {
				AppendHierarchyAabbOutlineLines(
				    lineOutlines, b, kHierarchySelectionOutlineColor, kHierarchySelectionOutlinePadPx);
			}
		}
		CollectHierarchyFallbackMarker(node, kHierarchySelectionOutlineColor, fallbackMarkers);
	}
} // namespace Engine
