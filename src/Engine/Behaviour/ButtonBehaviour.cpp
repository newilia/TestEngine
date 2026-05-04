#include "ButtonBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/Visual.h"

#include <SFML/Window/Mouse.hpp>

void ButtonBehaviour::OnEvent(const sf::Event& event) {
	auto window = Engine::MainContext::GetInstance().GetMainWindow();
	if (!window) {
		return;
	}

	const auto toWorld = [&](sf::Vector2i pixel) {
		return Utils::MapWindowPixelToWorld(*window, pixel);
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			if (HitTestWorld(toWorld(pressed->position))) {
				_mouseDown = true;
				_onTap();
			}
		}
		return;
	}

	if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (released->button == sf::Mouse::Button::Left && _mouseDown) {
			_mouseDown = false;
			_onRelease();
		}
		return;
	}

	if (const auto* began = event.getIf<sf::Event::TouchBegan>()) {
		if (!_touchDown && began->finger == 0) {
			if (HitTestWorld(toWorld(began->position))) {
				_touchDown = true;
				_touchFinger = began->finger;
				_onTap();
			}
		}
		return;
	}

	if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
		if (_touchDown && ended->finger == _touchFinger) {
			_touchDown = false;
			_onRelease();
		}
	}
}

Signal<>& ButtonBehaviour::GetOnTapSignal() const {
	return _onTap;
}

Signal<>& ButtonBehaviour::GetOnReleaseSignal() const {
	return _onRelease;
}

bool ButtonBehaviour::HitTestWorld(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	if (!node) {
		return false;
	}
	auto visual = node->GetVisual();
	if (!visual) {
		return false;
	}
	return visual->HitTest(worldPoint);
}
