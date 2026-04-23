#include "EngineInterface.h"

EngineContext::EngineContext() {
	_userInput = make_shared<UserInput>();
	_physicsHandler = make_shared<PhysicsHandler>();
	_fontManager = make_shared<FontManager>();
	_bodyPullHandler = make_shared<BodyPullHandler>();
}

void EngineContext::init() {}

sf::Time EngineContext::getSimDt() const {
	if (_isSimPaused) {
		return sf::Time();
	}
	auto dt = getFrameDt() * _simSpeedMultiplier;
	return dt;
}

sf::Time EngineContext::getFrameDt(bool ignoreFixed) const {
	if (!ignoreFixed && _fixedFrameTime) {
		return *_fixedFrameTime;
	}
	return _frameTime;
}

void EngineContext::onStartFrame() {
	_frameTime = _frameClock.getElapsedTime();
	_frameClock.restart();
}

void EngineContext::createMainWindow(sf::VideoMode mode, const sf::String& title, std::uint32_t /*style*/) {
	_mainWindow = std::make_shared<sf::RenderWindow>(mode, title, sf::State::Windowed);
}
