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
				tipPhysicsBody->AddVelocity(accVec * _shootAcceleration * deltaTime.asSeconds());
			}
			tipPhysicsBody->GetNode()->SetLocalRotation(_directionAngle);
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
				if (HitTestWorld(toWorld(pressed->position))) {
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

	void BilliardCueBehaviour::Activate() {
		GetNode()->SetEnabled(true);
	}

	void BilliardCueBehaviour::Deactivate() {
		GetNode()->SetEnabled(false);
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
		_isDragging = false;
		_isShooting = true;

		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			_tipCollideSubscription =
			    tipPhysicsBody->GetOnCollideSignal().Subscribe([this](const IntersectionDetails& intersection) {
				    OnTipCollide(intersection);
			    });
		}
		if (auto targetNode = _targetNode.Get()) {
			if (auto targetPhysicsBody = targetNode->FindBehaviour<PhysicsBodyBehaviour>()) {
				targetPhysicsBody->GetCollisionGroups().set(_collisionGroupOnShoot, true);
			}
		}
		if (auto tipPhysicsBody = _tipPhysicsBody.Get()) {
			tipPhysicsBody->SetLinearDamping(0.f);
		}
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

		targetPhysicsBody->GetCollisionGroups().set(_collisionGroupOnShoot, false);
		tipPhysicsBody->SetLinearDamping(_velocityDampingAfterHit);
		float cueVelocity = tipPhysicsBody->GetVelocity().length();
		float spinValue = _verticalSpin * _verticalSpinMultiplier * cueVelocity;
		rollingBallBehaviour->SetVerticalSpin(_directionAngle, -spinValue);

		_isShooting = false;
		_targetNode.Clear();
	}

	void BilliardCueBehaviour::SetBallRadius(float radius) {
		_ballRadius = radius;
	}
} // namespace Billiard
