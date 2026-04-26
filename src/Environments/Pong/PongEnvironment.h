#pragma once
#include "Engine/Core/Scene.h"

#include <memory>

class PongPlatform;

class PongEnvironment
{
public:
	void Setup();

private:
	std::shared_ptr<Scene> BuildScene();
	void AddWalls(Scene* scene);
	void AddBall(Scene* scene);
	void AddUserPlatform(Scene* scene);
	void AddAiPlatform(Scene* scene);
	std::shared_ptr<PongPlatform> CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
	                                                    sf::Color color) const;
	void ConfigureGlobalInput();
	void OnLose();
};
