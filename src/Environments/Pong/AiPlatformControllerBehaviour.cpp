#include "AiPlatformControllerBehaviour.h"

#include "AiPlatformControllerBehaviour.generated.hpp"
#include "PongPlatform.h"

#include <SFML/System/Time.hpp>

#include <algorithm>
#include <cstdlib>

void AiPlatformControllerBehaviour::OnInit() {
	Behaviour::OnInit();
	if (auto p = GetNode()) {
		_targetPos = p->GetPosGlobal();
		_defaultPos = p->GetPosGlobal();
	}
}

void AiPlatformControllerBehaviour::SetReactionDelay(sf::Time delay) {
	_reactionDelaySeconds = delay.asSeconds();
}

void AiPlatformControllerBehaviour::SetObservePeriod(sf::Time period) {
	_observePeriodSeconds = period.asSeconds();
}

void AiPlatformControllerBehaviour::SetAggressivenessParams(float aggression, sf::Time changePeriod) {
	_aggression = aggression;
	_aggressionChangePeriodSeconds = changePeriod.asSeconds();
}

void AiPlatformControllerBehaviour::BeginObserve(const std::weak_ptr<SceneNode>& opponentPlatformNode,
                                                 const std::weak_ptr<PongBall>& ball) {
	_opponentPlatform = opponentPlatformNode;
	_ball = ball;
}

void AiPlatformControllerBehaviour::ObserveState() {
	if (auto ball = _ball.lock()) {
		ExternalState state = {.ballPos = ball->GetShape()->getPosition()};
		_externalStateTimers.push({sf::Clock(), state});
	}
}

void AiPlatformControllerBehaviour::React() {
	MovePlatformTowardsBall();
}

void AiPlatformControllerBehaviour::MovePlatformTowardsBall() {
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto* selfCollider = node->FindShapeCollider();
	if (!selfCollider) {
		return;
	}
	sf::Shape* selfShape = selfCollider->GetBaseShape();
	float distanceToBall = 0.f;
	if (auto ball = _ball.lock()) {
		if (auto* ballShape = ball->GetShape()) {
			const sf::FloatRect selfBbox = selfCollider->GetBbox();
			distanceToBall =
			    std::abs(_curExState.ballPos.y - selfShape->getPosition().y) - ballShape->getRadius() - selfBbox.size.y;
		}
	}

	float steadyRatio = 0.f;
	if (auto opponentPlatform = _opponentPlatform.lock()) {
		if (auto* oppCollider = opponentPlatform->FindShapeCollider()) {
			if (sf::Shape* oppShape = oppCollider->GetBaseShape()) {
				const float distanceBetweenPlatforms = std::abs(selfShape->getPosition().y - oppShape->getPosition().y);
				if (distanceBetweenPlatforms > 0.f) {
					steadyRatio = 0.2f * std::clamp(distanceToBall / distanceBetweenPlatforms, 0.f, 1.f);
				}
			}
		}
	}

	_targetPos.x = _curExState.ballPos.x * (1.f - steadyRatio) + _defaultPos.x * steadyRatio;
}

void AiPlatformControllerBehaviour::OnUpdate(const sf::Time& /*dt*/) {
	auto node = GetNode();
	if (!node) {
		return;
	}

	if (_observeTimer.getElapsedTime().asSeconds() > _observePeriodSeconds) {
		_observeTimer.restart();
		ObserveState();
	}

	if (_aggressionChangeTimer.getElapsedTime().asSeconds() > _aggressionChangePeriodSeconds) {
		_aggressionChangeTimer.restart();
		constexpr float aggressionVariability = 0.5f;
		_aggression = _aggression * (1.f - aggressionVariability) +
		              static_cast<float>(std::rand() % 256) / 255.f * aggressionVariability;
	}

	if (!_externalStateTimers.empty()) {
		auto& stateTimer = _externalStateTimers.front();
		if (auto waitingTime = stateTimer.timePassed.getElapsedTime().asSeconds();
		    waitingTime > _reactionDelaySeconds) {
			_prevExState = _curExState;
			_curExState = stateTimer.state;
			_externalStateTimers.pop();
			React();
		}
	}

	ApplyPongPlatformVelocityTowardsTarget(node, _targetPos, _speedFactor, _velLimit);
}
