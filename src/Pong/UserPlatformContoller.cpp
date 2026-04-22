#include "UserPlatformContoller.h"

#include "Engine/EngineInterface.h"
#include "SFML/Window/Event.hpp"
#include "PongPlatform.h"

void UserPlatformController::init() {
	mTargetPos = mDefaultPos = mPlatform->getPosGlobal();
}

void UserPlatformController::handleEvent(const sf::Event& event) {
	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		float mouseYShift = static_cast<float>(moved->position.y) - mDefaultPos.y;
		float platformYShift = 0.f;
		for (int i = 0; i < std::abs(mouseYShift); ++i) {
			platformYShift += powf(mVerticalMoveFactor, static_cast<float>(i));
		}
		mTargetPos.x = static_cast<float>(moved->position.x);
		mTargetPos.y = mDefaultPos.y + copysignf(platformYShift, mouseYShift);
	}
}

void UserPlatformController::update(const sf::Time& dt) {
	auto vel = (mTargetPos - mPlatform->getShape()->getPosition()) * mSpeedFactor;
	vel.x = std::clamp(vel.x, -mVelLimit.x, mVelLimit.x);
	vel.y = std::clamp(vel.y, -mVelLimit.y, mVelLimit.y);
	mPlatform->getPhysicalComponent()->mVelocity = vel;
}
