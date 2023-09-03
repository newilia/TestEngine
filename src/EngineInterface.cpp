#include "EngineInterface.h"

EngineInterface::EngineInterface() {
	mSceneBuilder = make_shared<SceneBuilder>();
	mUserInput = make_shared<UserInput>();
	mPhysicsHandler = make_shared<PhysicsHandler>();
	mFontManager = make_shared<FontManager>();
	mBodyPullHandler = make_shared<BodyPullHandler>();
}

void EngineInterface::init() {
}

sf::Time EngineInterface::getSimDt() const {
	if (mIsSimPaused) {
		return sf::Time();
	}
	auto dt = mFrameTime * mSimSpeedMultiplier;
	return dt;
}

sf::Time EngineInterface::getFrameDt() const {
	return mFrameTime;
}

void EngineInterface::onStartFrame() {
	if (!mIsFixedFrameTime) {
		mFrameTime = mFrameClock.getElapsedTime();
	}
	mFrameClock.restart();
}
