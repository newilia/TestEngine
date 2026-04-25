#include "Engine/Editor/Editor.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/Core/Scene.h"

#include <SFML/Window/Event.hpp>

#include <imgui.h>

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
		return _sceneHierarchyWidget.GetSelected();
	}

	void Editor::ClearNodeSelection() {
		_sceneHierarchyWidget.ClearSelection();
	}

	void Editor::Draw() {
		if (!_isOpen) {
			return;
		}
		ImGui::SetNextWindowSize(ImVec2(320, 500), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Scene  [SceneHierarchyWidget]", nullptr, ImGuiWindowFlags_None)) {
			_sceneHierarchyWidget.Draw(EngineContext::Instance().GetScene());
		}
		ImGui::End();
		ImGui::SetNextWindowPos(ImVec2(360.0f, 0.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300.0f, 500.0f), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Inspector  [NodeInspectorWidget]", nullptr, ImGuiWindowFlags_None)) {
			_nodeInspectorWidget.Draw(GetSelectedNode());
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
		}
	}

	void Editor::OnKeyRelease(const sf::Keyboard::Key& /*key*/) {}

	void Editor::OnMouseMove(const sf::Vector2i& /*position*/) {}

} // namespace Engine
