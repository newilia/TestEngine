#pragma once
#include "BodyPullHandler.h"
#include "FontManager.h"
#include "Singleton.h"
#include "Scene.h"
#include "Physics/PhysicsHandler.h"
#include "UserInput.h"
#include "SceneBuilder.h"

class EngineInterface : public Singleton<EngineInterface> {
public:
	EngineInterface();
	~EngineInterface() override = default;
	void init();

	auto getSceneBuilder() { return mSceneBuilder; }
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
	sf::Time getFrameDt() const;
	void onStartFrame();
	void setMainWindow(sf::RenderWindow& window) { mMainWindow = &window; }
	sf::RenderWindow* getMainWindow() const { return mMainWindow; }
	void setFixedFrameTime(const sf::Time& time) { mFrameTime = time; mIsFixedFrameTime = true; }
	void resetFixedFrameTime() { mIsFixedFrameTime = false; }

private:
	sf::RenderWindow* mMainWindow;
	shared_ptr<SceneBuilder> mSceneBuilder;
	shared_ptr<Scene> mScene;
	shared_ptr<UserInput> mUserInput;
	shared_ptr<PhysicsHandler> mPhysicsHandler;
	shared_ptr<FontManager> mFontManager;
	shared_ptr<BodyPullHandler> mBodyPullHandler;
	sf::Clock mFrameClock;
	sf::Time mFrameTime;
	bool mIsFixedFrameTime = false;
	float mSimSpeedMultiplier = 1.f;
	bool mIsSimPaused = false;
};

inline EngineInterface* EI() { return EngineInterface::getInstance(); }