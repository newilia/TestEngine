#pragma once

#include "BilliardBallBehaviour.h"
#include "BilliardTurnIntent.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Signal.h"
#include "Engine/Visual/SpriteVisual.h"

#include <SFML/System/Angle.hpp>

namespace Billiard {

	class BilliardCueBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& deltaTime) override;

	public:
		[[nodiscard]] bool IsInteractable() const;
		[[nodiscard]] bool HitTestWorld(const sf::Vector2f& worldPoint) const;
		void AimAt(const sf::Vector2f& worldPoint);
		void ApplyShotIntent(const TurnIntent& intent);
		TurnIntent BuildTurnIntent(int playerIndex, std::uint32_t turnId) const;
		void Release();

		void SetCueBall(const std::shared_ptr<BilliardBallBehaviour>& ballBehaviour);
		void AbortAiming();
		void PrepareForNewTurn();

		Signal<>& GetOnReleaseSignal() const;
		Signal<>& GetOnHitSignal() const;
		Signal<>& GetOnAimChangedSignal() const;

		void SetLateralPosition(float position);
		void SetVerticalSpin(float spin);
		void SetDirectionAngle(sf::Angle angle);
		void SetDistanceFromTarget(float distance);
		[[nodiscard]] float GetDistanceFromTarget() const;
		void ResetDistanceFromTarget();
		void ApplyCueTransform();

		void PlayHideAnimation();
		void PlayShowAnimation();

		sf::Angle GetActualBallDirectionAngle() const;

	private:
		void OnTipCollide(const IntersectionDetails& intersection);
		std::shared_ptr<SceneNode> GetTargetBallNode() const;

	private:
		/// @property
		RefWrapper<PhysicsBodyBehaviour> _tipPhysicsBody;
		/// @property
		RefWrapper<SpriteVisual> _visual;
		/// @property(minValue=0, maxValue=7)
		int _overlapGroupOnRelease = 0;
		/// @property
		float _lateralPosition = 0.f;
		/// @property
		float _distanceFromTarget = 0.f;
		/// @property(setter=SetDirectionAngle)
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
		float _sideSpinBallRotationFactor = 0.5f;
		/// @property
		float _hideAnimationDuration = 0.75f;
		/// @property
		float _showAnimationDuration = 0.3f;
		/// @property
		float _ballRadius = 0.f;
		/// @property
		float _minDistanceFromTarget = 50.f;

	private:
		RefWrapper<BilliardBallBehaviour> _targetBall;
		bool _isShooting = false;
		float _verticalSpin = 0.f;
		float _distanceBeforeShoot = 0.f;
		Signal<const IntersectionDetails&>::Subscription _tipCollideSubscription;
		mutable Signal<> _onReleaseSignal;
		mutable Signal<> _onHitSignal;
		mutable Signal<> _onAimChangedSignal;
		float _animationProgress = 0.f;
		bool _isHiding = false;
		bool _isShowing = false;
	};

} // namespace Billiard
