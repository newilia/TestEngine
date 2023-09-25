#include "UserPlatformContoller.h"

#include "SFML/Window/Event.hpp"
#include "PongPlatform.h"

UserPlatformController::UserPlatformController(PongPlatform* platform)
	: mPlatform(platform)
{
}

void UserPlatformController::init() {
	mTargetPos = mDefaultPos = mPlatform->getPosGlobal();
}

void UserPlatformController::handleEvent(const sf::Event& event) {
	if (event.type == sf::Event::MouseMoved) {
		float mouseYShift = event.mouseMove.y - mDefaultPos.y;
		float platformYShift = 0.f;
		for (int i = 0; i < abs(mouseYShift); ++i) {
			platformYShift += pow(mVerticalMoveFactor, i);
		}
		mTargetPos = sf::Vector2f(event.mouseMove.x, mDefaultPos.y + copysignf(platformYShift, mouseYShift));
	}
}

void UserPlatformController::update(const sf::Time& dt) {
	auto vel = (mTargetPos - mPlatform->getShape()->getPosition()) * mSpeedFactor;
	vel.x = std::clamp(vel.x, -mMaxSpeed.x, mMaxSpeed.x);
	vel.y = std::clamp(vel.y, -mMaxSpeed.y, mMaxSpeed.y);
	mPlatform->getPhysicalComponent()->mVelocity = vel;
}
