#include "BilliardCueBehaviour.h"

#include "BilliardCueBehaviour.generated.hpp"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "LaunchProfiles/Billiard/RollingBallBehaviour.h"
#include "SFML/System/Angle.hpp"

#include <SFML/Window/Event.hpp>

#include <cmath>

namespace Billiard {

	void BilliardCueBehaviour::OnUpdate(const sf::Time& deltaTime) {
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			if (_isShooting) {
				sf::Vector2f accVec(std::cos(_directionAngle.asRadians()), std::sin(_directionAngle.asRadians()));
				const float accFactor =
				    (_cuePosition.x / _distanceBeforeShoot) * (_distanceBeforeShoot / _maxDistanceFromTarget);
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

	void BilliardCueBehaviour::OnEvent(const sf::Event& event) {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return;
		}
		const auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
			return Utils::MapWindowPixelToWorld(*window, pixel);
		};

		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
			if (pressed->button == sf::Mouse::Button::Left) {
				if (!_isShooting && !_isHiding && !_isShowing && HitTestWorld(toWorld(pressed->position))) {
					_isDragging = true;
				}
			}
			return;
		}

		if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
			if (_isDragging) {
				Aim(toWorld(moved->position));
			}
			return;
		}

		if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
			if (released->button == sf::Mouse::Button::Left) {
				if (_isDragging) {
					Release();
				}
			}
			return;
		}

		return;
	}

	void BilliardCueBehaviour::SetTargetNode(const std::shared_ptr<SceneNode>& node) {
		const auto cueNode = GetNode();
		if (!cueNode) {
			return;
		}
		_targetNode = node;
		_cuePosition = sf::Vector2f{};
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::SetDirection(sf::Angle angle) {
		_directionAngle = angle;
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::SetDistanceFromTarget(float distance) {
		_cuePosition.x = std::min(distance, _maxDistanceFromTarget);
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::MoveLongitudinal(float delta) {
		_cuePosition.x += delta;
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::SetLateralPosition(float position) {
		_cuePosition.y = position;
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::MoveLateral(float delta) {
		_cuePosition.y += delta;
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::SetCuePosition(const sf::Vector2f& cuePosition) {
		_cuePosition = cuePosition;
		ApplyCueTransform();
	}

	void BilliardCueBehaviour::ApplyCueTransform() {
		const auto cueNode = GetNode();
		const auto targetNode = _targetNode.Get();
		if (!cueNode || !targetNode) {
			return;
		}

		const sf::Vector2f cueOffset{-_cuePosition.x, _cuePosition.y * _ballRadius};
		const sf::Vector2f worldOffset = Utils::Rotate(cueOffset, _directionAngle.asRadians());
		const sf::Vector2f targetWorld = Utils::GetWorldPos(targetNode);

		cueNode->SetLocalRotation(_directionAngle);
		Utils::SetLocalPosToWorld(cueNode, targetWorld + worldOffset);
	}

	void BilliardCueBehaviour::SetVerticalSpin(float spin) {
		_verticalSpin = spin;
	}

	bool BilliardCueBehaviour::HitTestWorld(const sf::Vector2f& worldPoint) const {
		if (auto visual = _visual.Get()) {
			return visual->HitTest(worldPoint);
		}
		return false;
	}

	void BilliardCueBehaviour::Release() {
		_distanceBeforeShoot = _cuePosition.x;
		_isDragging = false;
		_isShooting = true;
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			_tipCollideSubscription =
			    tipPhysicsBody->GetOnOverlapSignal().Subscribe([this](const IntersectionDetails& intersection) {
				    OnTipCollide(intersection);
			    });
		}
		if (auto targetNode = _targetNode.Get()) {
			if (auto targetPhysicsBody = targetNode->FindBehaviour<PhysicsBodyBehaviour>()) {
				targetPhysicsBody->GetOverlapGroups().set(_overlapGroupOnShoot, true);
			}
		}
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			tipPhysicsBody->SetLinearDamping(0.f);
			tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnShoot, true);
		}

		_onReleaseSignal.Emit();
	}

	void BilliardCueBehaviour::Aim(const sf::Vector2f& worldPoint) {
		const auto targetNode = _targetNode.Get();
		if (!targetNode) {
			return;
		}

		const sf::Vector2f delta = Utils::GetWorldPos(targetNode) - worldPoint;
		const float distance = Utils::Length(delta);
		if (distance <= std::numeric_limits<float>::epsilon()) {
			return;
		}

		SetDirection(sf::radians(std::atan2(delta.y, delta.x)));
		SetDistanceFromTarget(distance);
	}

	void BilliardCueBehaviour::OnTipCollide(const IntersectionDetails& intersection) {
		auto tipPhysicsBody = _tipPhysicsBody.Get();
		auto targetNode = _targetNode.Get();
		if (!tipPhysicsBody || !targetNode) {
			return;
		}
		auto targetPhysicsBody = targetNode->FindBehaviour<PhysicsBodyBehaviour>();
		auto rollingBallBehaviour = targetNode->FindBehaviour<RollingBallBehaviour>();
		if (!targetPhysicsBody || !rollingBallBehaviour) {
			return;
		}

		targetPhysicsBody->GetOverlapGroups().set(_overlapGroupOnShoot, false);
		tipPhysicsBody->GetOverlapGroups().set(_overlapGroupOnShoot, false);
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
			    sf::Vector2f(v1.y, -v1.x).normalized() * _sideSpinBallDeflectionFactor * v1.length() * _cuePosition.y;
			tipPhysicsBody->SetVelocity(v1_final);
			targetPhysicsBody->SetVelocity(v2_final + ballDeflectionVector);

			float spinValue = _verticalSpin * _verticalSpinMultiplier * v1.length();
			rollingBallBehaviour->SetVerticalSpin(_directionAngle, -spinValue);
			targetPhysicsBody->SetAngularSpeed(-_cuePosition.y * _sideSpinBallRotationFactor);
		}

		_isShooting = false;
		_targetNode.Clear();

		_onHitSignal.Emit();
	}

	void BilliardCueBehaviour::SetBallRadius(float radius) {
		_ballRadius = radius;
	}

	Signal<>& BilliardCueBehaviour::GetOnReleaseSignal() const {
		return _onReleaseSignal;
	}

	Signal<>& BilliardCueBehaviour::GetOnHitSignal() const {
		return _onHitSignal;
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
		if (auto visual = _visual.Get()) {
			visual->SetColor(sf::Color(255, 255, 255, 0));
		}
	}
} // namespace Billiard
