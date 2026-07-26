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

	float BilliardBallBehaviour::GetRadius() const {
		if (auto ballShape = _ballShape.Get()) {
			return ballShape->GetRadius();
		}
		return 0.f;
	}

	void BilliardBallBehaviour::OnUpdate(const sf::Time& dt) {
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
} // namespace Billiard
