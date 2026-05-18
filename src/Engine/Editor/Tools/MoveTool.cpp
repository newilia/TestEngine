#include "MoveTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <imgui.h>

namespace {

	void ZeroMotion(PhysicsBodyBehaviour* rb) {
		if (!rb) {
			return;
		}
		rb->SetVelocity({});
		rb->SetAngularSpeed(0.f);
	}

} // namespace

MoveTool::MoveTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

bool MoveTool::ProcessEvent(const sf::Event& event) {
	auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
		if (auto mainWindow = Engine::MainContext::GetInstance().GetMainWindow()) {
			return Utils::MapWindowPixelToWorld(*mainWindow, pixel);
		}
		return sf::Vector2f();
	};

	auto tryGrab = [&](const sf::Vector2f& pos) -> bool {
		auto scene = Engine::MainContext::GetInstance().GetScene();
		auto picked = scene ? scene->FindTopMostNodeAtPoint(pos) : nullptr;
		if (!picked) {
			_dragging = false;
			_grabbedNode.reset();
			if (_onSelect) {
				_onSelect(nullptr);
			}
			return true;
		}
		if (_onSelect) {
			_onSelect(picked);
		}
		const sf::Vector2f nodePos = Utils::GetWorldPos(picked);
		_grabOffset = nodePos - pos;
		_grabbedNode = picked;
		_dragging = true;
		if (auto rb = picked->FindBehaviour<PhysicsBodyBehaviour>()) {
			ZeroMotion(rb.get());
			_wasBodyFixed = rb->IsFixed();
			rb->SetFixed(true);
		}
		return true;
	};

	auto moveTo = [&](const sf::Vector2f& pos) {
		auto node = _grabbedNode.lock();
		if (!node) {
			return;
		}
		Utils::SetLocalPosToWorld(node, pos + _grabOffset);
		if (auto rb = node->FindBehaviour<PhysicsBodyBehaviour>()) {
			ZeroMotion(rb.get());
		}
	};

	auto release = [this]() {
		_dragging = false;
		if (auto node = _grabbedNode.lock()) {
			if (auto rb = node->FindBehaviour<PhysicsBodyBehaviour>()) {
				rb->SetFixed(_wasBodyFixed);
			}
		}
		_grabbedNode.reset();
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			return tryGrab(toWorld(pressed->position));
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			return tryGrab(toWorld(touch->position));
		}
	}

	if (_dragging) {
		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				release();
				return true;
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				release();
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

void MoveTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Drag physical bodies with LMB; velocity is cleared while moving.");
}
