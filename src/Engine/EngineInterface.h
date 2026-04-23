#pragma once
#include "FontManager.h"
#include "Physics/BodyPullHandler.h"
#include "Physics/PhysicsHandler.h"
#include "Scene.h"
#include "Singleton.h"
#include "UserInput.h"

#include <cstdint>

class EngineContext : public Singleton<EngineContext>
{
public:
	[[nodiscard]] static EngineContext& instance() { return *getInstance(); }

	EngineContext();
	~EngineContext() override = default;
	void init();

	auto getScene() { return _scene; }

	void setScene(const shared_ptr<Scene>& scene) { _scene = scene; }

	auto getUserInput() { return _userInput; }

	auto getPhysicsHandler() { return _physicsHandler; }

	auto getFontManager() { return _fontManager; }

	auto getBodyPullHandler() { return _bodyPullHandler; }

	float getSimSpeedMultiplier() const { return _simSpeedMultiplier; }

	void setSimSpeedMultiplier(float val) { _simSpeedMultiplier = val; }

	bool isSimPaused() const { return _isSimPaused; }

	void setSimPaused(bool paused) { _isSimPaused = paused; }

	sf::Time getSimDt() const;
	sf::Time getFrameDt(bool ignoreFixed = false) const;
	void onStartFrame();
	void createMainWindow(sf::VideoMode mode, const sf::String& title, std::uint32_t style = 0);

	sf::RenderWindow* getMainWindow() const { return _mainWindow.get(); }

	void setFixedFrameTime(const sf::Time& time) { _fixedFrameTime = time; }

	void resetFixedFrameTime() { _fixedFrameTime.reset(); }

	bool isDebugEnabled() const { return _isDebugDrawEnabled; }

	void setDebugEnabled(bool enabled) { _isDebugDrawEnabled = enabled; }

private:
	shared_ptr<sf::RenderWindow> _mainWindow;
	shared_ptr<Scene> _scene;
	shared_ptr<UserInput> _userInput;
	shared_ptr<PhysicsHandler> _physicsHandler;
	shared_ptr<FontManager> _fontManager;
	shared_ptr<BodyPullHandler> _bodyPullHandler;
	sf::Clock _frameClock;
	sf::Time _frameTime;
	std::optional<sf::Time> _fixedFrameTime;
	float _simSpeedMultiplier = 1.f;
	bool _isSimPaused = false;
	bool _isDebugDrawEnabled = true;
};