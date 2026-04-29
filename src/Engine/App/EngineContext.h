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
	sf::Time GetSimDt() const;
	sf::Time GetFrameDt(bool ignoreFixed = false) const;
	void OnStartFrame();
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
	std::uint32_t _framerateLimit = 1000;
};
