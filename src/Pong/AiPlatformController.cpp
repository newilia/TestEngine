#include "AiPlatformController.h"

#include "PongPlatform.h"

AiPlatformController::AiPlatformController(PongPlatform* platform) : UserPlatformController(platform) {

}

void AiPlatformController::observeState() {
	if (auto ball = mBall.lock()) {
		ObservedState state = {
			.ballPos = ball->getShape()->getPosition()
		};
		mObservedStates.push(state);
	}
}

void AiPlatformController::update(const sf::Time& dt) {
	if (mObserveTimer.getElapsedTime() > mObservePeriod) {
		mObserveTimer.restart();
		observeState();
	}

	if (!mObservedStates.empty()) {
		auto& state = mObservedStates.front();
		auto waitingTime = state.waitingTime.getElapsedTime();
		if (waitingTime > mReactionDelay) {
			auto ball = mBall.lock();
			auto opponentPlatform = mOpponentPlatform.lock();
			if (ball && opponentPlatform) {
				float distanceToBall = abs(state.ballPos.y - mPlatform->getShape()->getPosition().y) - ball->getShape()->getRadius() - mPlatform->getBbox().height;
				float distanceBetweenPlatforms = abs(mPlatform->getShape()->getPosition().y - opponentPlatform->getShape()->getPosition().y);
				float steadyRatio = 0.2f * std::clamp(distanceToBall / distanceBetweenPlatforms, 0.f, 1.f);

				mTargetPos.x = state.ballPos.x * (1.f - steadyRatio) + mDefaultPos.x * steadyRatio;
			}

			mObservedStates.pop();
		}
	}
	UserPlatformController::update(dt);
}
