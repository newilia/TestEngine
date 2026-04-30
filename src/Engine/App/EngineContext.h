#pragma once
#include "Engine/App/FontManager.h"
#include "Engine/App/UserInput.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <cstdint>
#include <memory>
#include <optional>

class EngineContext : public Singleton<EngineContext>
{
public:
	EngineContext();
	~EngineContext() override;
	void Init();
	shared_ptr<Scene> GetScene() const;
	void SetScene(const shared_ptr<Scene>& scene);
	shared_ptr<UserInput> GetUserInput() const;
	shared_ptr<PhysicsProcessor> GetPhysicsProcessor() const;
	shared_ptr<FontManager> GetFontManager() const;
	float GetSimSpeedMultiplier() const;
	void SetSimSpeedMultiplier(float val);
	bool IsSimPaused() const;
	void SetSimPaused(bool paused);

	void OnStartPresentFrame();
	void OnStartUpdateTick();

	sf::Time GetSimTickDt() const;
	sf::Time GetWallTickDt() const;
	sf::Time GetFrameDt() const;

	/// Target logic update rate in Hz. 0 = one variable step per main-loop iteration (`wallDelta * speed`, capped).
	[[nodiscard]] std::uint32_t GetTargetTickRateHz() const;
	void SetTargetTickRateHz(std::uint32_t hz);

	void SetVerticalSyncEnabled(bool enabled);
	[[nodiscard]] bool IsVerticalSyncEnabled() const;
	std::shared_ptr<sf::RenderWindow> CreateMainWindow(sf::VideoMode mode, const sf::String& title,
	                                                   std::uint32_t style = sf::Style::Default,
	                                                   sf::State state = sf::State::Windowed);

	sf::RenderWindow* GetMainWindow() const;
	bool IsDebugDrawEnabled() const;
	void SetDebugEnabled(bool enabled);

	/// World-space force arrows: endpoint = pos + (m * a) * scale (inverse-square field on entities).
	float GetFieldForceDebugArrowScale() const;
	void SetFieldForceDebugArrowScale(float scale);

	void SetFramerateLimit(std::uint32_t maxFps);
	[[nodiscard]] std::uint32_t GetFramerateLimit() const;

	void SetFramerateLimitEnabled(bool enabled);
	bool IsFramerateLimitEnabled() const;

	[[nodiscard]] float GetFps() const;
	[[nodiscard]] float GetTickRate() const;

	/// Editor hierarchy selection propagated for viewport overlay (outline). Cleared each frame via editor.
	void SetHierarchySelectedForViewport(const shared_ptr<Scene>& node);
	[[nodiscard]] shared_ptr<Scene> GetHierarchySelectedForViewport() const;

private:
	void ApplyWindowFrameSettings();

	shared_ptr<sf::RenderWindow> _mainWindow;
	shared_ptr<Scene> _scene;
	shared_ptr<UserInput> _userInput;
	shared_ptr<PhysicsProcessor> _physicsProcessor;
	shared_ptr<FontManager> _fontManager;
	sf::Clock _frameClock;
	sf::Time _frameTime;
	sf::Clock _tickClock;
	sf::Time _tickTime;
	float _simSpeedMultiplier = 1.f;
	bool _isSimPaused = false;
	bool _isDebugDrawEnabled = false;
	float _fieldForceDebugArrowScale = 0.02f;
	std::uint32_t _framerateLimit = 100;
	bool _isFramerateLimitEnabled = false;
	bool _verticalSyncEnabled = false;
	std::uint32_t _targetTickRateHz = 500;
	float _fps = 0.f;
	float _tickRate = 0.f;
	bool _haveFps = false;
	bool _haveTickRate = false;
	std::weak_ptr<Scene> _hierarchySelectedForViewport;
};
