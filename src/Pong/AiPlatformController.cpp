#include "AiPlatformController.h"

#include "PongPlatform.h"

AiPlatformController::AiPlatformController(PongPlatform* platform) : UserPlatformController(platform) {
}

void AiPlatformController::beginObserve(const std::weak_ptr<PongPlatform>& opponentPlatform,
	const std::weak_ptr<PongBall>& ball)
{
	mOpponentPlatform = opponentPlatform;
	mBall = ball;
}

void AiPlatformController::setAggressivenessParams(float aggression, const sf::Time& changePeriod) {
	mAggression = aggression;
	mAggressionChangePeriod = changePeriod;
}

void AiPlatformController::observeState() {
	if (auto ball = mBall.lock()) {
		ExternalState state = {
			.ballPos = ball->getShape()->getPosition()
		};
		mExternalStateTimers.push({ sf::Clock(), state });
	}
}

void AiPlatformController::react() {
	movePlatformTowardsBall();
}

void AiPlatformController::movePlatformTowardsBall() {
	float distanceToBall = 0.f;
	if (auto ball = mBall.lock()) {
		distanceToBall = abs(mCurExState.ballPos.y - mPlatform->getShape()->getPosition().y) - ball->getShape()->getRadius() - mPlatform->getBbox().height;
	}

	float steadyRatio = 0.f;
	if (auto opponentPlatform = mOpponentPlatform.lock()) {
		float distanceBetweenPlatforms = abs(mPlatform->getShape()->getPosition().y - opponentPlatform->getShape()->getPosition().y);
		//this value affects on how much AI will move platform to center as the ball is far away from him
		steadyRatio = 0.2f * std::clamp(distanceToBall / distanceBetweenPlatforms, 0.f, 1.f);
	}

	mTargetPos.x = mCurExState.ballPos.x * (1.f - steadyRatio) + mDefaultPos.x * steadyRatio;
}

void AiPlatformController::update(const sf::Time& dt) {
	if (mObserveTimer.getElapsedTime() > mObservePeriod) {
		mObserveTimer.restart();
		observeState();
	}

	if (mAggressionChangeTimer.getElapsedTime() > mAggressionChangePeriod) {
		mAggressionChangeTimer.restart();
		constexpr float aggressionVariability = 0.5f;
		mAggression = mAggression * (1.f - aggressionVariability) + 
			static_cast<float>(std::rand() % 256) / 255.f * aggressionVariability;
	}

	if (!mExternalStateTimers.empty()) {
		auto& stateTimer = mExternalStateTimers.front();
		;
		if (auto waitingTime = stateTimer.timePassed.getElapsedTime();  waitingTime > mReactionDelay) {
			mPrevExState = mCurExState;
			mCurExState = stateTimer.state;
			mExternalStateTimers.pop();
			react();
		}
	}


	UserPlatformController::update(dt);
}