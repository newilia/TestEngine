#pragma once
#include "Engine/Core/Scene.h"

#include <memory>

namespace sf {
	class Text;
}

class PongEnvironment
{
public:
	void Setup();
	~PongEnvironment();

private:
	std::shared_ptr<Scene> BuildScene();
	void AddWalls(Scene* scene);
	void AddBall(Scene* scene);
	void AddUserPlatform(Scene* scene);
	void AddAiPlatform(Scene* scene);
	void AddScoreboard(Scene* scene);
	std::shared_ptr<SceneNode> CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
	                                                 sf::Color color) const;
	void ConfigureGlobalInput();
	void OnLose();
	void OnWin();
	void ResetRound();
	void UpdateScoreText();

	std::shared_ptr<sf::Text> _scoreText;
	int _userScore = 0;
	int _aiScore = 0;
};
