#include "Engine/Editor/Editor.h"

#include <imgui.h>
#include <SFML/Window/Event.hpp>

namespace Engine {

void Editor::Toggle() { _isOpen = !_isOpen; }

bool Editor::IsOpen() const { return _isOpen; }

void Editor::Update(float /*dt*/) {}

void Editor::Draw() {
	if (!_isOpen) {
		return;
	}
	ImGui::ShowDemoWindow();
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
