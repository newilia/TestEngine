#include "AiPlatformController.h"

#include "PongPlatform.h"

AiPlatformController::AiPlatformController(PongPlatform* platform) : UserPlatformController(platform) {}

void AiPlatformController::beginObserve(const std::weak_ptr<PongPlatform>& opponentPlatform,
                                        const std::weak_ptr<PongBall>& ball) {
	_opponentPlatform = opponentPlatform;
	_ball = ball;
}

void AiPlatformController::setAggressivenessParams(float aggression, const sf::Time& changePeriod) {
	_aggression = aggression;
	_aggressionChangePeriod = changePeriod;
}

void AiPlatformController::observeState() {
	if (auto ball = _ball.lock()) {
		ExternalState state = {.ballPos = ball->GetShape()->getPosition()};
		_externalStateTimers.push({sf::Clock(), state});
	}
}

void AiPlatformController::react() {
	movePlatformTowardsBall();
}

void AiPlatformController::movePlatformTowardsBall() {
	float distanceToBall = 0.f;
	if (auto ball = _ball.lock()) {
		distanceToBall = std::abs(_curExState.ballPos.y - _platform->GetShape()->getPosition().y) -
		                 ball->GetShape()->getRadius() - _platform->getBbox().size.y;
	}

	float steadyRatio = 0.f;
	if (auto opponentPlatform = _opponentPlatform.lock()) {
		float distanceBetweenPlatforms =
		    std::abs(_platform->GetShape()->getPosition().y - opponentPlatform->GetShape()->getPosition().y);
		// this value affects on how much AI will move platform to center as the ball is far away from him
		steadyRatio = 0.2f * std::clamp(distanceToBall / distanceBetweenPlatforms, 0.f, 1.f);
	}

	_targetPos.x = _curExState.ballPos.x * (1.f - steadyRatio) + _defaultPos.x * steadyRatio;
}

void AiPlatformController::Update(const sf::Time& dt) {
	if (_observeTimer.getElapsedTime() > _observePeriod) {
		_observeTimer.restart();
		observeState();
	}

	if (_aggressionChangeTimer.getElapsedTime() > _aggressionChangePeriod) {
		_aggressionChangeTimer.restart();
		constexpr float aggressionVariability = 0.5f;
		_aggression = _aggression * (1.f - aggressionVariability) +
		              static_cast<float>(std::rand() % 256) / 255.f * aggressionVariability;
	}

	if (!_externalStateTimers.empty()) {
		auto& stateTimer = _externalStateTimers.front();
		;
		if (auto waitingTime = stateTimer.timePassed.getElapsedTime(); waitingTime > _reactionDelay) {
			_prevExState = _curExState;
			_curExState = stateTimer.state;
			_externalStateTimers.pop();
			react();
		}
	}

	UserPlatformController::Update(dt);
}