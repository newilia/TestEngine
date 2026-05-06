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

		void SetRootNode(const std::weak_ptr<SceneNode>& rootNode);
		void SetGunNode(const std::weak_ptr<SceneNode>& gunNode);
		void SetScoreNode(const std::weak_ptr<SceneNode>& scoreNode);

		void StartNewGame();

	private:
		std::shared_ptr<SceneNode> CreateBallNode() const;
		void SetBallNode(const std::weak_ptr<SceneNode>& ballNode);
		void Shoot();
		void AttachBallToGun();
		void DetachBallFromGun();

	private:
		std::weak_ptr<SceneNode> _rootNode;
		std::weak_ptr<SceneNode> _ballNode;
		std::weak_ptr<SceneNode> _gunNode;
		std::weak_ptr<SceneNode> _scoreNode;
		float _ballSpeed = 1000.0f;
		float _ballMass = 1.0f;
		float _ballRestitution = 0.5f;
		float _ballRadius = 10.0f;
		sf::Color _ballColor = sf::Color::Red;
	};
} // namespace BallGame1
