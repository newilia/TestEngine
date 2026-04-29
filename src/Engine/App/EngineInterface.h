#pragma once
#include "Engine/App/FontManager.h"
#include "Engine/App/UserInput.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Simulation/PhysicsHandler.h"

#include <cstdint>
#include <memory>

class BodyPullHandler;

class EngineContext : public Singleton<EngineContext>
{
public:
	[[nodiscard]] static EngineContext& Instance() { return *GetInstance(); }

	EngineContext();
	~EngineContext() override;
	void Init();

	auto GetScene() { return _scene; }

	void SetScene(const shared_ptr<Scene>& scene);

	auto GetUserInput() { return _userInput; }

	auto GetPhysicsHandler() { return _physicsHandler; }

	auto GetFontManager() { return _fontManager; }

	auto GetBodyPullHandler() { return _bodyPullHandler; }

	void SetBodyPullHandler(const shared_ptr<BodyPullHandler>& handler) { _bodyPullHandler = handler; }

	float GetSimSpeedMultiplier() const { return _simSpeedMultiplier; }

	void SetSimSpeedMultiplier(float val) { _simSpeedMultiplier = val; }

	bool IsSimPaused() const { return _isSimPaused; }

	void SetSimPaused(bool paused) { _isSimPaused = paused; }

	sf::Time GetSimDt() const;
	sf::Time GetFrameDt(bool ignoreFixed = false) const;
	void OnStartFrame();
	std::shared_ptr<sf::RenderWindow> CreateMainWindow(sf::VideoMode mode, const sf::String& title,
	                                                   std::uint32_t style = sf::Style::Default,
	                                                   sf::State state = sf::State::Windowed);

	sf::RenderWindow* GetMainWindow() const { return _mainWindow.get(); }

	void SetFixedFrameTime(const sf::Time& time) { _fixedFrameTime = time; }

	void ResetFixedFrameTime() { _fixedFrameTime.reset(); }

	bool IsDebugEnabled() const { return _isDebugDrawEnabled; }

	void SetDebugEnabled(bool enabled) { _isDebugDrawEnabled = enabled; }

	/// World-space force arrows: endpoint = pos + (m * a) * scale (inverse-square field on entities).
	float GetFieldForceDebugArrowScale() const { return _fieldForceDebugArrowScale; }

	void SetFieldForceDebugArrowScale(float scale) { _fieldForceDebugArrowScale = scale >= 0.f ? scale : 0.f; }

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
	float _fieldForceDebugArrowScale = 0.02f;
};
