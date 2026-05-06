#pragma once
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "GunControllerBehaviour.h"

namespace BallGame1 {
	class GameControllerBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()
	public:
		void OnInit() override;
		void OnUpdate(const sf::Time& dt) override;
		void OnEvent(const sf::Event& event) override;

		void SetBallNode(const std::weak_ptr<SceneNode>& ballNode);
		void SetGunNode(const std::weak_ptr<SceneNode>& gunNode);

	private:
		void Shoot();
		std::shared_ptr<SceneNode> CreateBallNode() const;

	private:
		std::weak_ptr<SceneNode> _ballNode;
		std::weak_ptr<SceneNode> _gunNode;
		float _ballSpeed = 1000.0f;
		float _ballMass = 1.0f;
		float _ballRestitution = 0.5f;
		float _ballRadius = 10.0f;
		sf::Color _ballColor = sf::Color::Red;
	};
} // namespace BallGame1
