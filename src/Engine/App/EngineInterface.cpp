#include "EngineInterface.h"
#include "Engine/Behaviour/Physics/BodyPullHandler.h"

EngineContext::EngineContext() {
	_userInput = make_shared<UserInput>();
	_physicsHandler = make_shared<PhysicsHandler>();
	_fontManager = make_shared<FontManager>();
}

EngineContext::~EngineContext() = default;

void EngineContext::Init() {}

sf::Time EngineContext::GetSimDt() const {
	if (_isSimPaused) {
		return sf::Time();
	}
	auto dt = GetFrameDt() * _simSpeedMultiplier;
	return dt;
}

sf::Time EngineContext::GetFrameDt(bool ignoreFixed) const {
	if (!ignoreFixed && _fixedFrameTime) {
		return *_fixedFrameTime;
	}
	return _frameTime;
}

void EngineContext::OnStartFrame() {
	_frameTime = _frameClock.getElapsedTime();
	_frameClock.restart();
}

void EngineContext::CreateMainWindow(sf::VideoMode mode, const sf::String& title, std::uint32_t /*style*/) {
	_mainWindow = std::make_shared<sf::RenderWindow>(mode, title, sf::State::Windowed);
}
