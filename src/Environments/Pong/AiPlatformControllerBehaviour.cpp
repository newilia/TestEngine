#include "AiPlatformControllerBehaviour.h"

#include "AiPlatformControllerBehaviour.generated.hpp"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "PongPlatform.h"

#include <SFML/System/Time.hpp>

#include <algorithm>
#include <cstdlib>

void AiPlatformControllerBehaviour::SetMovementBounds(std::weak_ptr<SceneNode> movementRegionRect) {
	_movementBounds = std::move(movementRegionRect);
}

void AiPlatformControllerBehaviour::OnInit() {
	Behaviour::OnInit();
	ResyncSpawnFromNode();
}

void AiPlatformControllerBehaviour::ResyncSpawnFromNode() {
	if (auto p = GetNode()) {
		_targetPos = _defaultPos = p->GetPosGlobal();
		ClampPongPlatformDesiredCenter(_targetPos, false, p, _movementBounds);
		ClampPongPlatformDesiredCenter(_defaultPos, false, p, _movementBounds);
		p->SetPosGlobal(_defaultPos);
	}
}

void AiPlatformControllerBehaviour::ClearPendingObservations() {
	while (!_externalStateTimers.empty()) {
		_externalStateTimers.pop();
	}
	_observeTimer.restart();
	if (auto ball = _ball.lock()) {
		if (auto ballNode = ball->GetNode()) {
			_curExState.ballPos = ballNode->GetPosGlobal();
			_prevExState = _curExState;
		}
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
		if (auto ballNode = ball->GetNode()) {
			ExternalState state = {.ballPos = ballNode->GetPosGlobal()};
			_externalStateTimers.push({sf::Clock(), state});
		}
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
	auto* bodyBeh = node->FindBehaviour<PhysicsBodyBehaviour>().get();
	if (!bodyBeh) {
		return;
	}
	const sf::Vector2f selfPos = node->GetPosGlobal();
	float distanceToBall = 0.f;
	if (auto ball = _ball.lock()) {
		if (auto* ballShape = ball->GetShape()) {
			const sf::FloatRect selfBbox = bodyBeh->GetBbox();
			distanceToBall = std::abs(_curExState.ballPos.y - selfPos.y) - ballShape->getRadius() - selfBbox.size.y;
		}
	}

	float steadyRatio = 0.f;
	if (auto opponentPlatform = _opponentPlatform.lock()) {
		const float distanceBetweenPlatforms = std::abs(selfPos.y - opponentPlatform->GetPosGlobal().y);
		if (distanceBetweenPlatforms > 0.f) {
			steadyRatio = 0.2f * std::clamp(distanceToBall / distanceBetweenPlatforms, 0.f, 1.f);
		}
	}

	_targetPos.x = _curExState.ballPos.x * (1.f - steadyRatio) + _defaultPos.x * steadyRatio;
}

void AiPlatformControllerBehaviour::OnUpdate(const sf::Time& /*dt*/) {
	auto node = GetNode();
	if (!node) {
		return;
	}

	ClampPongPlatformToPlayfield(node, false, _movementBounds);

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

	ClampPongPlatformDesiredCenter(_targetPos, false, node, _movementBounds);

	ApplyPongPlatformVelocityTowardsTarget(node, _targetPos, _speedFactor, _velLimit);
}
