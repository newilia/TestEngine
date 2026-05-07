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

		void SetFieldNode(const std::weak_ptr<SceneNode>& fieldNode);
		void SetGunNode(const std::weak_ptr<SceneNode>& gunNode);
		void SetScoreNode(const std::weak_ptr<SceneNode>& scoreNode);

		void SetBallParameters(float mass, float restitution, float radius, const sf::Color& color);

		void StartNewGame();

	private:
		std::shared_ptr<SceneNode> CreateBallNode() const;
		void Shoot();
		void AttachBallToGun(const std::shared_ptr<SceneNode>& ballNode);
		void DetachBallFromGun();

	private:
		/// @property(dragSpeed=10.f)
		float _ballSpeed = 1000.0f;
		/// @property(dragSpeed=1.0f)
		float _ballMass = 1.0f;
		/// @property(dragSpeed=0.05f)
		float _ballRestitution = 0.5f;
		/// @property(dragSpeed=1.f)
		float _ballRadius = 10.0f;
		/// @property
		sf::Color _ballColor = sf::Color::Red;

	private:
		std::weak_ptr<SceneNode> _fieldNode;
		std::weak_ptr<SceneNode> _ballNode;
		std::weak_ptr<SceneNode> _gunNode;
		std::weak_ptr<SceneNode> _scoreNode;
	};
} // namespace BallGame1
