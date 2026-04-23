#include "EngineInterface.h"

EngineContext::EngineContext() {
	mUserInput = make_shared<UserInput>();
	mPhysicsHandler = make_shared<PhysicsHandler>();
	mFontManager = make_shared<FontManager>();
	mBodyPullHandler = make_shared<BodyPullHandler>();
}

void EngineContext::init() {
}

sf::Time EngineContext::getSimDt() const {
	if (mIsSimPaused) {
		return sf::Time();
	}
	auto dt = getFrameDt() * mSimSpeedMultiplier;
	return dt;
}

sf::Time EngineContext::getFrameDt(bool ignoreFixed) const {
	if (!ignoreFixed && mFixedFrameTime) {
		return *mFixedFrameTime;
	}
	return mFrameTime;
}

void EngineContext::onStartFrame() {
	mFrameTime = mFrameClock.getElapsedTime();
	mFrameClock.restart();
}

void EngineContext::createMainWindow(sf::VideoMode mode, const sf::String& title, std::uint32_t /*style*/) {
	mMainWindow = std::make_shared<sf::RenderWindow>(mode, title, sf::State::Windowed);
}
