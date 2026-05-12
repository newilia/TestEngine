#pragma once
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/MetaClass.h"

#include <functional>

namespace BallGame1 {
	class GameControllerBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()
	public:
		void OnInit() override;
		void OnDeinit() override;
		void OnUpdate(const sf::Time& dt) override;
		void OnEvent(const sf::Event& event) override;

		void SetFieldNode(const std::weak_ptr<SceneNode>& fieldNode);
		void SetGunNode(const std::weak_ptr<SceneNode>& gunNode);
		void SetScoreNode(const std::weak_ptr<SceneNode>& scoreNode);
		void SetCreateBallFunc(const std::function<std::shared_ptr<SceneNode>(void)>& func);
		void SetShootVelocity(float vel);

		void StartNewGame();

	private:
		std::shared_ptr<SceneNode> CreateBallNode() const;
		void Shoot();
		void AttachBallToGun(const std::shared_ptr<SceneNode>& ballNode);
		void DetachBallFromGun();

	private:
		std::function<std::shared_ptr<SceneNode>(void)> _createBall;
		std::weak_ptr<SceneNode> _fieldNode;
		std::weak_ptr<SceneNode> _ballNode;
		std::weak_ptr<SceneNode> _gunNode;
		std::weak_ptr<SceneNode> _scoreNode;
		float _shootVelocity = 0.f;
	};
} // namespace BallGame1
