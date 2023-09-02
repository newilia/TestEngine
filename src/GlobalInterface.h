#pragma once
#include "BodyPullHandler.h"
#include "FontManager.h"
#include "Singleton.h"
#include "Scene.h"
#include "Physics/PhysicsHandler.h"
#include "UserInput.h"
#include "SceneBuilder.h"

class GlobalInterface : public Singleton<GlobalInterface> {
public:
	GlobalInterface();
	~GlobalInterface() override = default;
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
	sf::Time getFrameDtAndReset();
	void setMainWindow(const sf::RenderWindow& window) { mMainWindow = &window; }
	sf::RenderWindow const* getMainWindow() const { return mMainWindow; }

private:
	sf::RenderWindow const* mMainWindow;
	shared_ptr<SceneBuilder> mSceneBuilder;
	shared_ptr<Scene> mScene;
	shared_ptr<UserInput> mUserInput;
	shared_ptr<PhysicsHandler> mPhysicsHandler;
	shared_ptr<FontManager> mFontManager;
	shared_ptr<BodyPullHandler> mBodyPullHandler;
	sf::Clock mFrameClock;
	float mSimSpeedMultiplier = 1.f;
	bool mIsSimPaused = false;
};