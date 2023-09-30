#pragma once
#include <memory>

#include "Engine/Scene.h"

class PongPlatform;

class PongEnvironment {
public:
	void setup();
private:
	// scene building
	std::shared_ptr<Scene> buildScene();
	void addWalls(Scene* scene);
	void addBall(Scene* scene);
	void addUserPlatform(Scene* scene);
	void addAiPlatform(Scene* scene);

	// helpers
	shared_ptr<PongPlatform> createDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg, sf::Color color) const;
	void configureGlobalInput();

	// game interactions
	void onLose();
};
