#include "BilliardCueBehaviour.h"

#include "BilliardCueBehaviour.generated.hpp"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "LaunchProfiles/Billiard/RollingBallBehaviour.h"
#include "SFML/System/Angle.hpp"

#include <SFML/Window/Event.hpp>

#include <cmath>

namespace Billiard {

	bool BilliardCueBehaviour::IsInteractable() const {
		return !_isShooting && !_isHiding && !_isShowing;
	}

	void BilliardCueBehaviour::AimAt(const sf::Vector2f& worldPoint) {
		const auto targetNode = GetTargetBallNode();
		if (!targetNode) {
			return;
		}

		const sf::Vector2f delta = Utils::GetWorldPos(targetNode) - worldPoint;
		if (Utils::Length(delta) <= std::numeric_limits<float>::epsilon()) {
			return;
		}

		SetDirectionAngle(sf::radians(std::atan2(-delta.y, -delta.x)));
	}

	void BilliardCueBehaviour::SetDistanceFromTarget(float distance) {
		_distanceFromTarget = std::clamp(distance, _minDistanceFromTarget, _maxDistanceFromTarget);
		ApplyCueTransform();
	}

	float BilliardCueBehaviour::GetDistanceFromTarget() const {
		return _distanceFromTarget;
	}

	void BilliardCueBehaviour::ApplyShotIntent(const TurnIntent& intent) {
		SetDirectionAngle(intent.directionAngle);
		SetLateralPosition(intent.lateralSpin);
		SetVerticalSpin(intent.verticalSpin);
		SetDistanceFromTarget(intent.pullDistance);
		Release();
	}

	TurnIntent BilliardCueBehaviour::BuildTurnIntent(int playerIndex, std::uint32_t turnId) const {
		TurnIntent intent;
		intent.phase = TurnIntentPhase::Shoot;
		intent.playerIndex = playerIndex;
		intent.turnId = turnId;
		intent.directionAngle = _directionAngle;
		intent.pullDistance = _distanceFromTarget;
		intent.lateralSpin = _lateralPosition;
		intent.verticalSpin = _verticalSpin;
		return intent;
	}

	void BilliardCueBehaviour::OnUpdate(const sf::Time& deltaTime) {
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			if (_isShooting) {
				sf::Vector2f accVec(std::cos(_directionAngle.asRadians()), std::sin(_directionAngle.asRadians()));
				const float accFactor =
				    (_distanceFromTarget / _distanceBeforeShoot) * (_distanceBeforeShoot / _maxDistanceFromTarget);
				tipPhysicsBody->AddVelocity(accVec * _shootAcceleration * accFactor * deltaTime.asSeconds());
			}
			tipPhysicsBody->GetNode()->SetLocalRotation(_directionAngle);
		}

		if (_isHiding) {
			_animationProgress = std::min(_animationProgress + deltaTime.asSeconds() / _hideAnimationDuration, 1.f);
			if (auto visual = _visual.Get()) {
				visual->SetColor(sf::Color(255, 255, 255, 255 * (1.f - _animationProgress)));
			}
			if (_animationProgress >= 1.f) {
				_isHiding = false;
				GetNode()->SetEnabled(false);
			}
		}
		else if (_isShowing) {
			_animationProgress = std::min(_animationProgress + deltaTime.asSeconds() / _showAnimationDuration, 1.f);
			if (auto visual = _visual.Get()) {
				visual->SetColor(sf::Color(255, 255, 255, 255 * _animationProgress));
			}
			if (_animationProgress >= 1.f) {
				_isShowing = false;
			}
		}
	}

	void BilliardCueBehaviour::SetCueBall(const std::shared_ptr<BilliardBallBehaviour>& ballBehaviour) {
		_targetBall = ballBehaviour;
		if (ballBehaviour) {
			_ballRadius = ballBehaviour->GetRadius();
		}
		ResetDistanceFromTarget();
	}

	void BilliardCueBehaviour::SetLateralPosition(float position) {
		_lateralPosition = position;
		ApplyCueTransform();
		_onAimChangedSignal.Emit();
	}

	void BilliardCueBehaviour::ApplyCueTransform() {
		const auto cueNode = GetNode();
		const auto targetNode = GetTargetBallNode();
		if (!cueNode || !targetNode) {
			return;
		}

		const sf::Vector2f cueOffset{-_distanceFromTarget, _lateralPosition * _ballRadius};
		const sf::Vector2f worldOffset = Utils::Rotate(cueOffset, _directionAngle.asRadians());
		const sf::Vector2f targetWorld = Utils::GetWorldPos(targetNode);

		cueNode->SetLocalRotation(_directionAngle);
		Utils::SetLocalPosToWorld(cueNode, targetWorld + worldOffset);
	}

	void BilliardCueBehaviour::SetVerticalSpin(float spin) {
		_verticalSpin = spin;
	}

	void BilliardCueBehaviour::SetDirectionAngle(sf::Angle angle) {
		_directionAngle = angle;
		ApplyCueTransform();
		_onAimChangedSignal.Emit();
	}

	bool BilliardCueBehaviour::HitTestWorld(const sf::Vector2f& worldPoint) const {
		if (auto visual = _visual.Get()) {
			return visual->HitTest(worldPoint);
		}
		return false;
	}

	void BilliardCueBehaviour::Release() {
		if (_distanceFromTarget <= _minDistanceFromTarget) {
			return;
		}

		_distanceBeforeShoot = _distanceFromTarget;
		_isShooting = true;
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			_tipCollideSubscription =
			    tipPhysicsBody->GetOnOverlapSignal().Subscribe([this](const IntersectionDetails& intersection) {
				    OnTipCollide(intersection);
			    });
		}
		if (auto targetNode = GetTargetBallNode()) {
			if (auto targetPhysicsBody = targetNode->FindBehaviour<PhysicsBodyBehaviour>()) {
				targetPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, true);
			}
		}
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			tipPhysicsBody->SetLinearDamping(0.f);
			tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, true);
		}

		_onReleaseSignal.Emit();
	}

	void BilliardCueBehaviour::OnTipCollide(const IntersectionDetails& intersection) {
		if (!_isShooting) {
			return;
		}

		auto tipPhysicsBody = _tipPhysicsBody.Get();
		auto targetNode = GetTargetBallNode();
		if (!tipPhysicsBody || !targetNode) {
			return;
		}
		auto targetPhysicsBody = targetNode->FindBehaviour<PhysicsBodyBehaviour>();
		auto rollingBallBehaviour = targetNode->FindBehaviour<RollingBallBehaviour>();
		if (!targetPhysicsBody || !rollingBallBehaviour) {
			return;
		}

		targetPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, false);
		tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, false);
		tipPhysicsBody->SetLinearDamping(_velocityDampingAfterHit);

		auto v1 = tipPhysicsBody->GetVelocity();
		if (v1.lengthSquared() > 1e-6) {
			auto m1 = tipPhysicsBody->GetMass();
			auto m2 = targetPhysicsBody->GetMass();
			constexpr auto v2 = sf::Vector2f();
			auto restitution = std::min(targetPhysicsBody->GetRestitution(), tipPhysicsBody->GetRestitution());
			auto v1_final = v1 - ((1 + restitution) * m2) / (m1 + m2) * (v1 - v2);
			auto v2_final = v2 - ((1 + restitution) * m1) / (m1 + m2) * (v2 - v1);
			const auto ballDeflectionVector =
			    sf::Vector2f(v1.y, -v1.x).normalized() * _sideSpinBallDeflectionFactor * v1.length() * _lateralPosition;
			tipPhysicsBody->SetVelocity(v1_final);
			targetPhysicsBody->SetVelocity(v2_final + ballDeflectionVector);

			float spinValue = _verticalSpin * _verticalSpinMultiplier * v1.length();
			rollingBallBehaviour->SetVerticalSpin(_directionAngle, -spinValue);
			targetPhysicsBody->SetAngularSpeed(-_lateralPosition * _sideSpinBallRotationFactor);
		}

		_isShooting = false;
		_onHitSignal.Emit();
	}

	sf::Angle BilliardCueBehaviour::GetActualBallDirectionAngle() const {
		return _directionAngle;
	}

	std::optional<sf::Vector2f> BilliardCueBehaviour::GetTargetBallWorldPosition() const {
		const auto targetNode = GetTargetBallNode();
		if (!targetNode) {
			return std::nullopt;
		}
		return Utils::GetWorldPos(targetNode);
	}

	Signal<>& BilliardCueBehaviour::GetOnReleaseSignal() const {
		return _onReleaseSignal;
	}

	Signal<>& BilliardCueBehaviour::GetOnHitSignal() const {
		return _onHitSignal;
	}

	Signal<>& BilliardCueBehaviour::GetOnAimChangedSignal() const {
		return _onAimChangedSignal;
	}

	void BilliardCueBehaviour::PlayHideAnimation() {
		_isHiding = true;
		_isShowing = false;
		_animationProgress = 0.f;
	}

	void BilliardCueBehaviour::PlayShowAnimation() {
		_isShowing = true;
		_isHiding = false;
		_animationProgress = 0.f;
		GetNode()->SetEnabled(true);
		GetNode()->SetVisible(true);
		if (auto visual = _visual.Get()) {
			visual->SetColor(sf::Color(255, 255, 255, 0));
		}
	}

	std::shared_ptr<SceneNode> BilliardCueBehaviour::GetTargetBallNode() const {
		if (!_targetBall) {
			return nullptr;
		}
		return _targetBall.Get()->GetNode();
	}

	void BilliardCueBehaviour::AbortAiming() {
		_isShooting = false;

		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, false);
		}
	}

	void BilliardCueBehaviour::ResetDistanceFromTarget() {
		SetDistanceFromTarget(_minDistanceFromTarget);
	}

	void BilliardCueBehaviour::PrepareForNewTurn() {
		_isShooting = false;
		ResetDistanceFromTarget();
		PlayShowAnimation();

		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			tipPhysicsBody->SetVelocity(sf::Vector2f());
			tipPhysicsBody->SetAngularSpeed(0.f);
			tipPhysicsBody->SetLinearDamping(0.f);
			tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnRelease, false);
		}
	}

} // namespace Billiard
