#pragma once

#include "BilliardBallBehaviour.h"
#include "Engine/Behaviour/EventHandlerBehaviourBase.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/SpriteVisual.h"

#include <SFML/System/Angle.hpp>

namespace Billiard {

	class BilliardCueBehaviour : public EventHandlerBehaviourBase
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& deltaTime) override;
		void OnEvent(const sf::Event& event) override;

	public:
		void SetTargetNode(const std::shared_ptr<SceneNode>& node);
		void Aim(const sf::Vector2f& pointerWorldPoint);

		void SetDirection(sf::Angle direction);
		void SetDistanceFromTarget(float distance);
		void MoveLongitudinal(float delta);
		void SetLateralPosition(float position);
		void MoveLateral(float delta);
		void SetVerticalSpin(float spin);
		void SetCuePosition(const sf::Vector2f& cuePosition);
		void SetBallRadius(float radius);
		Signal<>& GetOnReleaseSignal() const;
		Signal<>& GetOnHitSignal() const;

		void PlayHideAnimation();
		void PlayShowAnimation();

	private:
		void Release();
		bool HitTestWorld(const sf::Vector2f& worldPoint) const;
		void OnTipCollide(const IntersectionDetails& intersection);

		void ApplyCueTransform();

	private:
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _tipPhysicsBody;
		/// @property
		RefWrapper<SpriteVisual> _visual;
		/// @property(minValue=0, maxValue=7)
		int _overlapGroupOnShoot = 0;
		/// @property
		RefWrapper<SceneNode> _targetNode;
		/// @property(setter=SetCuePosition)
		sf::Vector2f _cuePosition{};
		/// @property(setter=SetDirection)
		sf::Angle _directionAngle{};
		/// @property
		float _shootAcceleration = 5.f;
		/// @property
		float _velocityDampingAfterHit = 500.f;
		/// @property
		float _maxDistanceFromTarget = 500.f;
		/// @property
		float _verticalSpinMultiplier = 1.f;
		/// @property
		float _sideSpinBallDeflectionFactor = 1.f;
		/// @property
		float _sideSpinBallRotationFactor = 1.f;
		/// @property
		float _hideAnimationDuration = 1.5f;
		/// @property
		float _showAnimationDuration = 0.3f;

	private:
		float _ballRadius = 0.f;
		bool _isDragging = false;
		bool _isShooting = false;
		float _verticalSpin = 0.f;
		float _distanceBeforeShoot = 0.f;
		Signal<const IntersectionDetails&>::Subscription _tipCollideSubscription;
		mutable Signal<> _onReleaseSignal;
		mutable Signal<> _onHitSignal;
		float _animationProgress = 0.f;
		bool _isHiding = false;
		bool _isShowing = false;
	};

} // namespace Billiard
