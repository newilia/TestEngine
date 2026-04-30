#include "Engine/Editor/Editor.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui_internal.h>

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
		ImGuiID id_right = 0;
		ImGuiID id_bottom = 0;
		ImGuiID id_left_top = 0;
		ImGuiID id_left_bottom = 0;
		ImGuiID id_main = dockspace_id;
		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Left, 0.22f, &id_left, &id_main);
		ImGui::DockBuilderSplitNode(id_left, ImGuiDir_Down, 0.32f, &id_left_bottom, &id_left_top);
		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Right, 0.30f, &id_right, &id_main);
		ImGui::DockBuilderSplitNode(id_main, ImGuiDir_Down, 0.20f, &id_bottom, &id_main);

		ImGui::DockBuilderDockWindow(kSceneWindowTitle, id_left_top);
		ImGui::DockBuilderDockWindow(kToolsWindowTitle, id_left_bottom);
		ImGui::DockBuilderDockWindow(kInspectorWindowTitle, id_right);
		ImGui::DockBuilderDockWindow(kDebugWindowTitle, id_bottom);
		ImGui::DockBuilderFinish(dockspace_id);
		layout_finished = true;
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

	void Editor::Update(float /*dt*/) {
		Engine::MainContext::GetInstance().SetHierarchySelectedForViewport(_isOpen ? _sceneHierarchyWidget.GetSelected()
		                                                                           : nullptr);
	}

	std::shared_ptr<SceneNode> Editor::GetSelectedNode() const {
		return _sceneHierarchyWidget.GetSelected();
	}

	void Editor::ClearNodeSelection() {
		_sceneHierarchyWidget.ClearSelection();
	}

	void Editor::SetSelectedNode(std::shared_ptr<SceneNode> node) {
		_sceneHierarchyWidget.Select(std::move(node));
	}

	EditorToolManager& Editor::GetEditorToolManager() {
		return *_editorToolManager;
	}

	const EditorToolManager& Editor::GetEditorToolManager() const {
		return *_editorToolManager;
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
			_nodeInspectorWidget.Draw(GetSelectedNode());
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
		if (const auto* e = event.getIf<sf::Event::Resized>()) {
			OnResize(e->size);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
			OnKeyPress(e->code);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::KeyReleased>()) {
			OnKeyRelease(e->code);
			return;
		}
		if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
			OnMouseMove(e->position);
		}
	}

	void Editor::OnResize(const sf::Vector2u& /*size*/) {}

	void Editor::OnKeyPress(const sf::Keyboard::Key& key) {
		if (key == sf::Keyboard::Key::F1) {
			Toggle();
			return;
		}
		if (key == sf::Keyboard::Key::Space) {
			MainContext::GetInstance().ToggleSimPaused();
		}
		if (_isOpen && !ImGui::GetIO().WantCaptureKeyboard && GetEditorToolManager().TryActivateToolViaDigitKey(key)) {
			return;
		}
	}

	void Editor::OnKeyRelease(const sf::Keyboard::Key& /*key*/) {}

	void Editor::OnMouseMove(const sf::Vector2i& /*position*/) {}
} // namespace Engine
