#pragma once
#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "Environments/EnvironmentBase.h"

#include <memory>

namespace sf {
	class Text;
}

class SceneNode;

class PongEnvironment : public EnvironmentBase, public Engine::EventHandlerBase
{
public:
	~PongEnvironment() override;
	void Setup() override;
	void OnEvent(const sf::Event& event) override;

private:
	std::shared_ptr<Scene> BuildScene();
	void AddWalls(Scene* scene);
	void AddBall(Scene* scene, float radius);
	void AddUserPlatform(Scene* scene);
	void AddAiPlatform(Scene* scene);
	void AddScoreboard(Scene* scene);
	std::shared_ptr<SceneNode> CreateDefaultPlatform(sf::Vector2f size, sf::Vector2f pos, float rotationDeg,
	                                                 sf::Color color) const;
	void OnLose();
	void OnWin();
	void ResetRound();
	void UpdateScoreText();

	std::shared_ptr<sf::Text> _scoreText;
	std::shared_ptr<SceneNode> _scoreboardNode;
	int _userScore = 0;
	int _aiScore = 0;
};
