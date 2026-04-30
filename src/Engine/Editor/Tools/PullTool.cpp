#include "PullTool.h"

#include "Engine/App/EngineContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/Scene.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>

namespace {

	constexpr float kBasePullStrength = 100000.f;

} // namespace

PullTool::PullTool(SelectTool::SelectCallback onSelect) : _onSelect(std::move(onSelect)) {}

UserPullBehaviour::PullMode PullTool::PullModeFromIndex(int index) {
	switch (std::clamp(index, 0, 2)) {
	case 0:
		return UserPullBehaviour::PullMode::POSITION;
	case 1:
		return UserPullBehaviour::PullMode::FORCE;
	default:
		return UserPullBehaviour::PullMode::VELOCITY;
	}
}

void PullTool::SetArrowVisual(std::shared_ptr<VectorArrowVisual> arrow) {
	_arrowVisual = std::move(arrow);
}

std::shared_ptr<SceneNode> PullTool::StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode) {
	auto physicsProcessor = Engine::MainContext::GetInstance().GetPhysicsProcessor();
	if (!physicsProcessor) {
		return nullptr;
	}
	for (auto wBody : physicsProcessor->GetAllBodies()) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto* collider = body->FindShapeCollider();
		if (!collider || !Utils::IsPointInsideOfBody(mousePos, collider)) {
			continue;
		}
		auto rigidBody = body->FindBehaviour<RigidBodyBehaviour>();
		if (rigidBody && rigidBody->IsImmovable()) {
			if (pullMode == UserPullBehaviour::PullMode::FORCE || pullMode == UserPullBehaviour::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->RequireBehaviour<UserPullBehaviour>();
		pullComponent->_localSourcePoint = mousePos - body->GetPosGlobal();
		pullComponent->_globalDestPoint = mousePos;
		pullComponent->_mode = pullMode;
		pullComponent->_pullingStrength = kBasePullStrength * _pullForceScale;
		_pullingBody = body;
		return body;
	}
	return nullptr;
}

void PullTool::StopPull() {
	if (auto pullingBody = _pullingBody.lock()) {
		pullingBody->RemoveBehaviour<UserPullBehaviour>();
	}
	_pullingBody.reset();
}

void PullTool::SetPullDestination(sf::Vector2f dest) const {
	if (auto pullingBody = _pullingBody.lock()) {
		if (auto pullComp = pullingBody->FindBehaviour<UserPullBehaviour>()) {
			pullComp->_globalDestPoint = dest;
		}
	}
}

bool PullTool::processEvent(const sf::Event& event) {
	auto toVec = [](sf::Vector2i p) {
		return sf::Vector2f(static_cast<float>(p.x), static_cast<float>(p.y));
	};

	if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (pressed->button == sf::Mouse::Button::Left) {
			const auto pos = toVec(pressed->position);
			_isDragging = true;
			_onSelect(StartPull(pos, PullModeFromIndex(_pullModeIndex)));
			return true;
		}
	}
	if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
		if (touch->finger == 0) {
			const auto pos = sf::Vector2f(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y));
			_isDragging = true;
			_onSelect(StartPull(pos, PullModeFromIndex(_pullModeIndex)));
			return true;
		}
	}

	if (_isDragging) {
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
			SetPullDestination(toVec(moved->position));
			return true;
		}
		if (const auto* touch = event.getIf<sf::Event::TouchMoved>()) {
			if (touch->finger == 0) {
				SetPullDestination(
				    sf::Vector2f(static_cast<float>(touch->position.x), static_cast<float>(touch->position.y)));
				return true;
			}
		}
	}

	return false;
}

void PullTool::onPresent(const sf::Time& /*dt*/) {
	auto arrow = _arrowVisual.lock();
	if (!arrow) {
		return;
	}
	if (!_debugArrowEnabled) {
		arrow->SetVisible(false);
		return;
	}
	if (auto body = _pullingBody.lock()) {
		if (auto pullComp = body->FindBehaviour<UserPullBehaviour>()) {
			if (pullComp->_mode == UserPullBehaviour::PullMode::FORCE) {
				arrow->SetColor(sf::Color::Green);
				arrow->SetStartPos(body->GetPosGlobal() + pullComp->_localSourcePoint);
				arrow->SetEndPos(pullComp->_globalDestPoint);
				arrow->SetVisible(true);
				return;
			}
		}
	}
	arrow->SetVisible(false);
}

PullVisualSetup CreatePullVisualOverlay() {
	auto root = std::make_shared<SceneNode>();
	root->SetName("body_pull");

	auto arrowNode = std::make_shared<SceneNode>();
	arrowNode->SetName("body_pull_arrow");
	auto arrowVis = std::make_shared<VectorArrowVisual>();
	arrowNode->SetVisual(arrowVis);
	root->AddChild(std::move(arrowNode));

	return PullVisualSetup{std::move(root), std::move(arrowVis)};
}
