#include "UserPlatformContoller.h"

#include "EngineInterface.h"
#include "SFML/Window/Event.hpp"
#include "PongPlatform.h"

void UserPlatformController::init() {
	mTargetPos = mDefaultPos = mPlatform->getPosGlobal();
}

void UserPlatformController::handleEvent(const sf::Event& event) {
	if (event.type == sf::Event::MouseMoved) {
		float mouseYShift = event.mouseMove.y - mDefaultPos.y;
		float platformYShift = 0.f;
		for (int i = 0; i < abs(mouseYShift); ++i) {
			platformYShift += powf(mVerticalMoveFactor, static_cast<float>(i));
		}
		mTargetPos.x = static_cast<float>(event.mouseMove.x);
		mTargetPos.y = mDefaultPos.y + copysignf(platformYShift, mouseYShift);
	}
}

void UserPlatformController::update(const sf::Time& dt) {
	auto vel = (mTargetPos - mPlatform->getShape()->getPosition()) * mSpeedFactor;
	vel.x = std::clamp(vel.x, -mVelLimit.x, mVelLimit.x);
	vel.y = std::clamp(vel.y, -mVelLimit.y, mVelLimit.y);
	mPlatform->getPhysicalComponent()->mVelocity = vel;
}
