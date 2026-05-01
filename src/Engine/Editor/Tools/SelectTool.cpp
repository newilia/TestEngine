#include "SelectTool.h"

#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Core/Scene.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

SelectTool::SelectTool(SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

bool SelectTool::processEvent(const sf::Event& event) {
	auto applyPick = [this](const sf::Vector2f& worldPoint) -> bool {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = scene ? scene->FindTopMostNodeAtPoint(worldPoint) : nullptr;
		_onSelect(picked);
		return true;
	};

	auto* window = Engine::MainContext::GetInstance().GetMainWindow();
	if (!window) {
		return false;
	}

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			return applyPick(Utils::MapWindowPixelToWorld(*window, pressed->position));
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return applyPick(Utils::MapWindowPixelToWorld(*window, touch->position));
		}
	}
	return false;
}
