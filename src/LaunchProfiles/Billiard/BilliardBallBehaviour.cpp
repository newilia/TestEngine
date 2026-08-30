#include "BilliardBallBehaviour.h"

#include "BilliardBallBehaviour.generated.hpp"

namespace Billiard {

	void BilliardBallBehaviour::SetBallNumber(int ballNumber) {
		_ballNumber = ballNumber;
	}

	int BilliardBallBehaviour::GetBallNumber() const {
		return _ballNumber;
	}

	bool BilliardBallBehaviour::IsCue() const {
		return _ballNumber == 0;
	}

	bool BilliardBallBehaviour::IsEight() const {
		return _ballNumber == 8;
	}

	bool BilliardBallBehaviour::IsStriped() const {
		return _ballNumber >= 9 && _ballNumber <= 15;
	}

	void BilliardBallBehaviour::PlayFallAnimation() {
		_isFalling = true;
		_fallAnimationProgress = 0.f;

		if (auto lightReceiver = _lightReceiver.Get()) {
			_initialLightingStrength = lightReceiver->GetLightingStrength();
		}
	}

	void BilliardBallBehaviour::Appear() {
		if (GetNode()->IsEnabled()) {
			return;
		}
		GetNode()->SetEnabled(true);
		_isFalling = false;
		_fallAnimationProgress = 0.f;
		if (auto lightReceiver = _lightReceiver.Get()) {
			lightReceiver->SetLightingStrength(_initialLightingStrength);
		}
		if (auto textureContributor = _textureContributor.Get()) {
			textureContributor->SetTint(sf::Color(255, 255, 255, 255));
		}
	}

	float BilliardBallBehaviour::GetRadius() const {
		if (auto ballShape = _ballShape.Get()) {
			return ballShape->GetRadius();
		}
		return 0.f;
	}

	std::shared_ptr<PhysicsBodyBehaviour> BilliardBallBehaviour::GetPhysicsBody() const {
		return _physicsBody.Get();
	}

	std::shared_ptr<RollingBallBehaviour> BilliardBallBehaviour::GetRollingBallBehaviour() const {
		return _rollingBall.Get();
	}

	void BilliardBallBehaviour::OnUpdate(const sf::Time& dt) {
		TryInitializeCollisionGroups();

		if (_isFalling) {
			_fallAnimationProgress += dt.asSeconds() / _fallAnimationDuration;

			if (_fallAnimationProgress >= 1.f) {
				_fallAnimationProgress = 1.f;
				_isFalling = false;
				GetNode()->SetEnabled(false);
			}

			if (auto lightReceiver = _lightReceiver.Get()) {
				lightReceiver->SetLightingStrength(_initialLightingStrength * (1.f - _fallAnimationProgress));
			}
			if (auto textureContributor = _textureContributor.Get()) {
				textureContributor->SetTint(sf::Color(255, 255, 255, 255 * (1.f - _fallAnimationProgress)));
			}
		}
	}

	void BilliardBallBehaviour::RestoreCollisionGroups() {
		if (auto physicsBody = _physicsBody.Get()) {
			physicsBody->GetCollisionGroups() = _defaultCollisionGroups;
			physicsBody->GetOverlapGroups() = _defaultOverlapGroups;
		}
	}

	void BilliardBallBehaviour::TryInitializeCollisionGroups() {
		if (_isCollisionGroupsInitialized) {
			return;
		}
		if (auto physicsBody = _physicsBody.Get()) {
			_defaultOverlapGroups = physicsBody->GetOverlapGroups();
			_defaultCollisionGroups = physicsBody->GetCollisionGroups();
			_isCollisionGroupsInitialized = true;
		}
	}
} // namespace Billiard
