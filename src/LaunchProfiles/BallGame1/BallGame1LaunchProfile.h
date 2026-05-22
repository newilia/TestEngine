#pragma once

#include "Engine/Core/EventHandlerBase.h"
#include "Engine/Core/Scene.h"
#include "LaunchProfiles/LaunchProfileBase.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

class SceneNode;

namespace BallGame1 {

	class BallGame1LaunchProfile : public LaunchProfileBase, public Engine::EventHandlerBase
	{
	public:
		~BallGame1LaunchProfile() override = default;
		void Setup() override;
		void OnEvent(const sf::Event& event) override;

	private:
		std::shared_ptr<Scene> BuildScene();
		std::shared_ptr<SceneNode> CreateFieldNode();
		std::shared_ptr<SceneNode> CreateGunNode();
		std::shared_ptr<SceneNode> CreateScoreNode();
		std::shared_ptr<SceneNode> CreateBallNode();

		const sf::Vector2f _fieldSize{1000, 1000};
	};

} // namespace BallGame1
