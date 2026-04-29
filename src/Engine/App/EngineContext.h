#pragma once
#include "Engine/App/FontManager.h"
#include "Engine/App/UserInput.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Simulation/PhysicsHandler.h"

#include <cstdint>
#include <memory>
#include <optional>

class BodyPullHandler;

class EngineContext : public Singleton<EngineContext>
{
public:
	EngineContext();
	~EngineContext() override;
	void Init();
	shared_ptr<Scene> GetScene() const;
	void SetScene(const shared_ptr<Scene>& scene);
	shared_ptr<UserInput> GetUserInput() const;
	shared_ptr<PhysicsHandler> GetPhysicsHandler() const;
	shared_ptr<FontManager> GetFontManager() const;
	shared_ptr<BodyPullHandler> GetBodyPullHandler() const;
	void SetBodyPullHandler(const shared_ptr<BodyPullHandler>& handler);
	float GetSimSpeedMultiplier() const;
	void SetSimSpeedMultiplier(float val);
	bool IsSimPaused() const;
	void SetSimPaused(bool paused);
	/// Last per-tick simulation step (fixed `1/targetHz` or variable when unlimited). Zero if no tick this frame.
	sf::Time GetSimDt() const;
	sf::Time GetFrameDt(bool ignoreFixed = false) const;
	void OnStartFrame();

	/// Wall-clock frame delta (ignores debug fixed frame dt). Use for logic accumulator and HUD FPS.
	sf::Time GetRawFrameDt() const;

	/// Target logic update rate in Hz. 0 = one variable step per frame (`GetRawFrameDt` * speed, capped).
	[[nodiscard]] std::uint32_t GetTargetTickRateHz() const;
	void SetTargetTickRateHz(std::uint32_t hz);

	/// Logic ticks executed in the current frame (after `RunSimulationTicksForFrame` / main loop sim block).
	[[nodiscard]] unsigned GetLogicTicksLastFrame() const;

	void SetVerticalSyncEnabled(bool enabled);
	[[nodiscard]] bool IsVerticalSyncEnabled() const;
	std::shared_ptr<sf::RenderWindow> CreateMainWindow(sf::VideoMode mode, const sf::String& title,
	                                                   std::uint32_t style = sf::Style::Default,
	                                                   sf::State state = sf::State::Windowed);

	sf::RenderWindow* GetMainWindow() const;
	void SetFixedFrameTime(const sf::Time& time);
	void ResetFixedFrameTime();
	[[nodiscard]] std::optional<sf::Time> GetFixedFrameTime() const;
	bool IsDebugEnabled() const;
	void SetDebugEnabled(bool enabled);

	/// World-space force arrows: endpoint = pos + (m * a) * scale (inverse-square field on entities).
	float GetFieldForceDebugArrowScale() const;
	void SetFieldForceDebugArrowScale(float scale);

	/// SFML window cap (FPS). 0 disables limiting (`setFramerateLimit`).
	[[nodiscard]] std::uint32_t GetFramerateLimit() const;

	void SetFramerateLimit(std::uint32_t maxFps);

	/// Call once per frame before stepping simulation: advances accumulator when not paused; clears last-step dt.
	void BeginLogicFrame();

	/// Try to consume one fixed logic step of `stepSeconds` from the accumulator. Returns false if not enough time.
	bool TryConsumeLogicAccumulator(double stepSeconds);

	/// Sets the simulation step dt for behaviours / `GetSimDt` until the next tick (main loop only).
	void SetLastLogicStepDt(const sf::Time& dt);

	void SetLogicTicksLastFrame(unsigned n);

private:
	void ApplyWindowFrameSettings();

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
	bool _isDebugDrawEnabled = false;
	float _fieldForceDebugArrowScale = 0.02f;
	std::uint32_t _framerateLimit = 0;
	bool _verticalSyncEnabled = true;
	std::uint32_t _targetTickRateHz = 200;
	double _logicAccumulatorSec = 0.;
	sf::Time _lastLogicStepDt;
	unsigned _logicTicksLastFrame = 0;
};
