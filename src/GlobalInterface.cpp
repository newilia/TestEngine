#include "GlobalInterface.h"

GlobalInterface::GlobalInterface() {
	mSceneBuilder = make_shared<SceneBuilder>();
	mUserInput = make_shared<UserInput>();
	mPhysicsHandler = make_shared<PhysicsHandler>();
	mFontManager = make_shared<FontManager>();
	mBodyPullHandler = make_shared<BodyPullHandler>();
}

sf::Time GlobalInterface::getFrameDtAndReset() {
	if (mIsSimPaused) {
		return sf::Time();
	}
	auto dt = mFrameClock.getElapsedTime() * mSimSpeedMultiplier;
	mFrameClock.restart();
	return dt;
}
