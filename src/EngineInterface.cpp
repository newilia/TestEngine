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
	auto dt = getFrameDt() * mSimSpeedMultiplier;
	return dt;
}

sf::Time EngineInterface::getFrameDt(bool ignoreFixed) const {
	if (!ignoreFixed && mFixedFrameTime) {
		return *mFixedFrameTime;
	}
	return mFrameTime;
}

void EngineInterface::onStartFrame() {
	mFrameTime = mFrameClock.getElapsedTime();
	mFrameClock.restart();
}
