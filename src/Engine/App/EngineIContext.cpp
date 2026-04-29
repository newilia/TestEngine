#include "Engine/Behaviour/Physics/BodyPullHandler.h"
#include "EngineContext.h"

shared_ptr<Scene> EngineContext::GetScene() const {
	return _scene;
}

shared_ptr<UserInput> EngineContext::GetUserInput() const {
	return _userInput;
}

shared_ptr<PhysicsHandler> EngineContext::GetPhysicsHandler() const {
	return _physicsHandler;
}

shared_ptr<FontManager> EngineContext::GetFontManager() const {
	return _fontManager;
}

shared_ptr<BodyPullHandler> EngineContext::GetBodyPullHandler() const {
	return _bodyPullHandler;
}

void EngineContext::SetBodyPullHandler(const shared_ptr<BodyPullHandler>& handler) {
	_bodyPullHandler = handler;
}

float EngineContext::GetSimSpeedMultiplier() const {
	return _simSpeedMultiplier;
}

void EngineContext::SetSimSpeedMultiplier(float val) {
	_simSpeedMultiplier = val;
}

bool EngineContext::IsSimPaused() const {
	return _isSimPaused;
}

void EngineContext::SetSimPaused(bool paused) {
	_isSimPaused = paused;
}

sf::RenderWindow* EngineContext::GetMainWindow() const {
	return _mainWindow.get();
}

void EngineContext::SetFixedFrameTime(const sf::Time& time) {
	_fixedFrameTime = time;
}

void EngineContext::ResetFixedFrameTime() {
	_fixedFrameTime.reset();
}

std::optional<sf::Time> EngineContext::GetFixedFrameTime() const {
	return _fixedFrameTime;
}

bool EngineContext::IsDebugEnabled() const {
	return _isDebugDrawEnabled;
}

void EngineContext::SetDebugEnabled(bool enabled) {
	_isDebugDrawEnabled = enabled;
}

float EngineContext::GetFieldForceDebugArrowScale() const {
	return _fieldForceDebugArrowScale;
}

void EngineContext::SetFieldForceDebugArrowScale(float scale) {
	_fieldForceDebugArrowScale = scale >= 0.f ? scale : 0.f;
}

std::uint32_t EngineContext::GetFramerateLimit() const {
	return _framerateLimit;
}

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
