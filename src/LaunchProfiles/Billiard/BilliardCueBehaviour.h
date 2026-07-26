#pragma once

#include "BilliardBallBehaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Signal.h"

#include <SFML/System/Angle.hpp>

namespace Billiard {

	class BilliardCueBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& deltaTime) override;
		void OnEvent(const sf::Event& event) override;

	public:
		void Activate();
		void Deactivate();
		void SetTargetNode(const std::shared_ptr<SceneNode>& node);
		void Aim(const sf::Vector2f& pointerWorldPoint);

		void SetDirection(sf::Angle direction);
		void SetDistanceFromTarget(float distance);
		void MoveLongitudinal(float delta);
		void SetLateralPosition(float position);
		void MoveLateral(float delta);
		void SetVerticalSpin(float spin);
		void SetCuePosition(const sf::Vector2f& cuePosition);

	private:
		void Release();
		bool HitTestWorld(const sf::Vector2f& worldPoint) const;
		void OnTipCollide(const IntersectionDetails& intersection);

		void ApplyCueTransform();

	private:
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _tipPhysicsBody;
		/// @property
		RefWrapper<Visual> _visual;
		/// @property(minValue=0, maxValue=7)
		int _collisionGroupOnShoot = 0;
		/// @property
		RefWrapper<SceneNode> _targetNode;
		/// @property(setter=SetCuePosition)
		sf::Vector2f _cuePosition{};
		/// @property(setter=SetDirection)
		sf::Angle _direction{};
		/// @property
		float _shootAcceleration = 5.f;
		/// @property
		float _velocityDampingAfterHit = 1000.f;
		/// @property
		float _maxDistanceFromTarget = 500.f;

	private:
		bool _isDragging = false;
		bool _isShooting = false;
		float _verticalSpin = 0.f;
		Signal<const IntersectionDetails&>::Subscription _tipCollideSubscription;
	};

} // namespace Billiard
