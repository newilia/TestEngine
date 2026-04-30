#include "MoveTool.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Editor/Tools/ScenePickUtils.h"

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
	auto toVec = [](sf::Vector2i p) {
		return sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y));
	};

	auto tryBegin = [&](sf::Vector2f pos) -> bool {
		auto scene = EngineContext::GetInstance().GetScene();
		auto picked = PickSceneNodeAt(scene, pos);
		if (!picked) {
			_dragging = false;
			_grabbed.reset();
			_onSelect(nullptr);
			return true;
		}
		_onSelect(picked);
		if (auto rb = picked->FindBehaviour<RigidBodyBehaviour>()) {
			if (rb->IsImmovable()) {
				return true;
			}
		}
		const sf::Vector2f nodePos = picked->GetPosGlobal();
		_grabOffset = nodePos - pos;
		_grabbed = picked;
		_dragging = true;
		if (auto rb = picked->FindBehaviour<RigidBodyBehaviour>()) {
			ZeroMotion(rb.get());
		}
		return true;
	};

	auto moveTo = [&](sf::Vector2f pos) {
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
			return tryBegin(toVec(pressed->position));
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return tryBegin(sf::Vector2f(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y)));
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
			moveTo(toVec(moved->position));
			return true;
		}
		if (const auto* tm = event.getIf<sf::Event::TouchMoved>()) {
			if (tm->finger == 0) {
				moveTo(sf::Vector2f(static_cast<float>(tm->position.x), static_cast<float>(tm->position.y)));
				return true;
			}
		}
	}

	return false;
}
