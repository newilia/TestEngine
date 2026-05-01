#include "MoveTool.h"

#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/Scene.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

namespace {

	void ZeroMotion(RigidBodyBehaviour* rb) {
		if (!rb) {
			return;
		}
		rb->_velocity = {};
		rb->_angularSpeed = 0.f;
	}

} // namespace

MoveTool::MoveTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

bool MoveTool::processEvent(const sf::Event& event) {
	auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
		if (auto mainWindow = Engine::MainContext::GetInstance().GetMainWindow()) {
			return Utils::MapWindowPixelToWorld(*mainWindow, pixel);
		}
		return sf::Vector2f();
	};

	auto tryBegin = [&](const sf::Vector2f& pos) -> bool {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = scene ? scene->FindTopMostNodeAtPoint(pos) : nullptr;
		if (!picked) {
			_dragging = false;
			_grabbed.reset();
			_onSelect(nullptr);
			return true;
		}
		_onSelect(picked);
		const sf::Vector2f nodePos = picked->GetPosGlobal();
		_grabOffset = nodePos - pos;
		_grabbed = picked;
		_dragging = true;
		if (auto rb = picked->FindBehaviour<RigidBodyBehaviour>()) {
			ZeroMotion(rb.get());
		}
		return true;
	};

	auto moveTo = [&](const sf::Vector2f& pos) {
		auto node = _grabbed.lock();
		if (!node) {
			return;
		}
		node->SetPosGlobal(pos + _grabOffset);
		if (auto rb = node->FindBehaviour<RigidBodyBehaviour>()) {
			ZeroMotion(rb.get());
		}
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			return tryBegin(toWorld(pressed->position));
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return tryBegin(toWorld(touch->position));
		}
	}

	if (_dragging) {
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				_dragging = false;
				_grabbed.reset();
				return true;
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				_dragging = false;
				_grabbed.reset();
				return true;
			}
		}
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			moveTo(toWorld(moved->position));
			return true;
		}
		if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
			if (tm->finger == 0) {
				moveTo(toWorld(tm->position));
				return true;
			}
		}
	}

	return false;
}

void MoveTool::onPresent(const sf::Time& dt) {
	if (auto node = _grabbed.lock()) {
		if (auto rb = node->FindBehaviour<RigidBodyBehaviour>()) {
			ZeroMotion(rb.get());
		}
	}
}
