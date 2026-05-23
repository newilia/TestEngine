#pragma once
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Core/AssetRef.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneObject.h"

namespace BallGame1 {
	class GameControllerBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()
	public:
		void OnInit() override;
		void OnDeinit() override;
		void OnUpdate(const sf::Time& dt) override;
		void OnEvent(const sf::Event& event) override;

		/// @method
		void StartNewGame();

	private:
		std::shared_ptr<SceneNode> CreateBallNode() const;
		void Shoot();
		void AttachBallToGun(const std::shared_ptr<SceneNode>& ballNode);
		void DetachBallFromGun();

	private:
		/// @property
		AssetRef<SceneObject> _ballAsset;
		/// @property
		RefWrapper<SceneNode> _fieldNode;
		/// @property
		RefWrapper<SceneNode> _gunNode;
		/// @property
		RefWrapper<SceneNode> _scoreNode;
		/// @property(dragSpeed=0.05f)
		float _shootVelocity = 100.f;

	private:
		std::weak_ptr<SceneNode> _ballNode;
	};
} // namespace BallGame1
