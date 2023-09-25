#pragma once
#include "BodyPullHandler.h"
#include "FontManager.h"
#include "Singleton.h"
#include "Scene.h"
#include "Physics/PhysicsHandler.h"
#include "UserInput.h"
#include "TestEnvironment.h"

class EngineInterface : public Singleton<EngineInterface> {
public:
	EngineInterface();
	~EngineInterface() override = default;
	void init();
	
	auto getScene() { return mScene; }
	void setScene(const shared_ptr<Scene>& scene) { mScene = scene; }
	auto getUserInput() { return mUserInput; }
	auto getPhysicsHandler() { return mPhysicsHandler; }
	auto getFontManager() { return mFontManager; }
	auto getBodyPullHandler() { return mBodyPullHandler; }
	float getSimSpeedMultiplier() const { return mSimSpeedMultiplier; }
	void setSimSpeedMultiplier(float val) { mSimSpeedMultiplier = val; }
	bool isSimPaused() const { return mIsSimPaused; }
	void setSimPaused(bool paused) { mIsSimPaused = paused; }
	sf::Time getSimDt() const;
	sf::Time getFrameDt(bool ignoreFixed = false) const;
	void onStartFrame();
	void createMainWindow(sf::VideoMode mode, const sf::String& title, sf::Uint32 style = sf::Style::Default, const sf::ContextSettings& settings = sf::ContextSettings());
	sf::RenderWindow* getMainWindow() const { return mMainWindow.get(); }
	void setFixedFrameTime(const sf::Time& time) { mFixedFrameTime = time; }
	void resetFixedFrameTime() { mFixedFrameTime.reset(); }
	bool isDebugEnabled() const { return mIsDebugDrawEnabled; }
	void setDebugEnabled(bool enabled) { mIsDebugDrawEnabled = enabled; }

private:
	shared_ptr<sf::RenderWindow> mMainWindow;
	shared_ptr<Scene> mScene;
	shared_ptr<UserInput> mUserInput;
	shared_ptr<PhysicsHandler> mPhysicsHandler;
	shared_ptr<FontManager> mFontManager;
	shared_ptr<BodyPullHandler> mBodyPullHandler;
	sf::Clock mFrameClock;
	sf::Time mFrameTime;
	std::optional<sf::Time> mFixedFrameTime;
	float mSimSpeedMultiplier = 1.f;
	bool mIsSimPaused = false;
	bool mIsDebugDrawEnabled = true;
};

inline EngineInterface* EI() { return EngineInterface::getInstance(); }