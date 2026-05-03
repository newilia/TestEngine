#include "PullTool.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>

namespace {
	constexpr float kBasePullStrength = 100000.f;
} // namespace

PullTool::PullTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

void PullTool::SetPullForceScale(float v) {
	_pullForceScale = v;
}

float PullTool::GetPullForceScale() const {
	return _pullForceScale;
}

bool PullTool::IsDebugArrowEnabled() const {
	return _debugArrowEnabled;
}

void PullTool::SetDebugArrowEnabled(bool v) {
	_debugArrowEnabled = v;
}

std::shared_ptr<SceneNode> PullTool::OnTap(const sf::Vector2f& screenPixelPos) {
	auto physicsProcessor = Engine::MainContext::GetInstance().GetPhysicsProcessor();
	if (!physicsProcessor) {
		return nullptr;
	}

	auto window = Engine::MainContext::GetInstance().GetMainWindow();
	if (!window) {
		return nullptr;
	}

	const auto worldMousePos = Utils::MapWindowPixelToWorld(*window, screenPixelPos);

	SetPullDestination(worldMousePos);

	/* TODO use universal Visual::HitTest */
	for (const auto& wBody : physicsProcessor->GetAllBodies()) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto rigidBody = body->FindBehaviour<PhysicsBodyBehaviour>();
		if (!rigidBody) {
			continue;
		}
		if (!Utils::IsWorldPointInsideOfBody(worldMousePos, rigidBody.get())) {
			continue;
		}
		if (rigidBody->IsImmovable()) {
			continue;
		}
		_pullingBody = body;
		return body;
	}
	return nullptr;
}

void PullTool::StopPull() {
	_pullingBody.reset();
}

void PullTool::SetPullDestination(const sf::Vector2f& destination) {
	_destination = destination;
}

bool PullTool::processEvent(const sf::Event& event) {
	auto toVec2f = [](sf::Vector2i p) {
		return sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y));
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			const auto pos = toVec2f(pressed->position);
			_isDragging = true;
			_onSelect(OnTap(pos));
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			const auto pos = sf::Vector2f(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y));
			_isDragging = true;
			_onSelect(OnTap(pos));
			return true;
		}
	}

	if (_isDragging) {
		auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
			if (auto mainWindow = Engine::MainContext::GetInstance().GetMainWindow()) {
				return Utils::MapWindowPixelToWorld(*mainWindow, pixel);
			}
			return sf::Vector2f();
		};

		if (event.is<sf::Event::MouseButtonReleased>()) {
			if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
				if (released->button == sf::Mouse::Button::Left) {
					_isDragging = false;
					StopPull();
					return true;
				}
			}
		}
		if (const auto* ended = event.getIf<sf::Event::TouchEnded>()) {
			if (ended->finger == 0) {
				_isDragging = false;
				StopPull();
				return true;
			}
		}
		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			SetPullDestination(toWorld(moved->position));
			return true;
		}
		if (const auto* touch = event.getIf<sf::Event::TouchMoved>()) {
			if (touch->finger == 0) {
				SetPullDestination(toWorld(touch->position));
				return true;
			}
		}
	}

	return false;
}

void PullTool::onPresent(const sf::Time& dt) {
	auto body = _pullingBody.lock();
	if (!body) {
		return;
	}
	_arrow.SetColor(sf::Color::Green);
	_arrow.SetStartPos(body->GetPosGlobal());
	_arrow.SetEndPos(_destination);

	if (auto rigidBody = body->FindBehaviour<PhysicsBodyBehaviour>()) {
		auto pullVector = _destination - body->GetPosGlobal();
		auto distance = Utils::Length(pullVector);
		if (distance <= std::numeric_limits<float>::epsilon()) {
			return;
		}

		auto force = pullVector * kBasePullStrength * _pullForceScale;
		rigidBody->AddVelocity(force * dt.asSeconds() / rigidBody->GetMass());
	}
}

void PullTool::drawOverlay(sf::RenderWindow& window) {
	if (!_debugArrowEnabled) {
		return;
	}
	if (_pullingBody.expired()) {
		return;
	}
	_arrow.draw(window, sf::RenderStates::Default);
}
