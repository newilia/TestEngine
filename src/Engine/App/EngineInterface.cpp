#include "EngineInterface.h"

#include "Engine/Behaviour/Physics/BodyPullHandler.h"

EngineContext::EngineContext() {
	_userInput = make_shared<UserInput>();
	_physicsHandler = make_shared<PhysicsHandler>();
	_fontManager = make_shared<FontManager>();
}

EngineContext::~EngineContext() {
	SetScene(nullptr);
}

void EngineContext::SetScene(const shared_ptr<Scene>& scene) {
	if (_scene) {
		_scene->NotifyLifecycleDeinitRecursive();
	}
	_scene = scene;
	if (_scene) {
		_scene->NotifyLifecycleInitRecursive();
	}
}

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

std::shared_ptr<sf::RenderWindow> EngineContext::CreateMainWindow(sf::VideoMode mode, const sf::String& title,
                                                                  std::uint32_t style, sf::State state) {
	_mainWindow = std::make_shared<sf::RenderWindow>(mode, title, style, state);
	_mainWindow->setFramerateLimit(_framerateLimit);
	return _mainWindow;
}

void EngineContext::SetFramerateLimit(std::uint32_t maxFps) {
	_framerateLimit = maxFps;
	if (_mainWindow) {
		_mainWindow->setFramerateLimit(maxFps);
	}
}
