#include "SelectTool.h"

#include "Engine/App/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Editor/Tools/ScenePickUtils.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

SelectTool::SelectTool(SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

bool SelectTool::processEvent(const sf::Event& event) {
	auto applyPick = [this](sf::Vector2f pos) -> bool {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = PickSceneNodeAt(scene, pos);
		_onSelect(picked);
		return true;
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			return applyPick(
			    sf::Vector2f(static_cast<float>(pressed->position.x), static_cast<float>(pressed->position.y)));
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return applyPick(
			    sf::Vector2f(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y)));
		}
	}
	return false;
}
