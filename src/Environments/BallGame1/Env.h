#pragma once

#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "Environments/EnvironmentBase.h"

#include <memory>

class SceneNode;
class TextVisual;

namespace BallGame1 {

	class Env : public EnvironmentBase, public Engine::EventHandlerBase
	{
	public:
		void Setup() override;
		void OnEvent(const sf::Event& event) override;

	private:
		std::shared_ptr<Scene> BuildScene();
		std::shared_ptr<SceneNode> CreateBackgroundNode();
		std::shared_ptr<SceneNode> CreateFieldNode();
		std::shared_ptr<SceneNode> CreateBallNode();
		std::shared_ptr<SceneNode> CreateStartNode();
		std::shared_ptr<SceneNode> CreateScoreNode();

	private:
		const sf::Vector2f _fieldSize{1000, 1000};
	};

} // namespace BallGame1
