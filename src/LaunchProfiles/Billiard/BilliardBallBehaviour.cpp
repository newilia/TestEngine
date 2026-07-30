#include "BilliardBallBehaviour.h"

#include "BilliardBallBehaviour.generated.hpp"

#include <Engine/Core/SfmlWindowUtils.h>

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
		_isFalling = false;
		_fallAnimationProgress = 0.f;
		GetNode()->SetEnabled(true);
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

	void BilliardBallBehaviour::OnEvent(const sf::Event& event) {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return;
		}
		const auto toWorld = [&](sf::Vector2i pixel) -> sf::Vector2f {
			return Utils::MapWindowPixelToWorld(*window, pixel);
		};

		if (auto e = event.getIf<sf::Event::MouseButtonPressed>()) {
			auto worldPos = toWorld(e->position);
			if (auto visual = _ballShape.Get()) {
				if (visual->HitTest(worldPos)) {
					_dragStartPosition = worldPos;
				}
			}
		}
		else if (auto e = event.getIf<sf::Event::MouseMoved>()) {
			if (_dragStartPosition && _allowedFreeMoveArea.has_value()) {
				auto newPos = GetNode()->GetLocalPosition();
				auto pointerWorldPos = toWorld(e->position);
				auto delta = pointerWorldPos - *_dragStartPosition;
				newPos += delta;
				auto radius = GetRadius();
				newPos.x = std::clamp(newPos.x, _allowedFreeMoveArea->position.x + radius,
				    _allowedFreeMoveArea->position.x + _allowedFreeMoveArea->size.x - radius);
				newPos.y = std::clamp(newPos.y, _allowedFreeMoveArea->position.y + radius,
				    _allowedFreeMoveArea->position.y + _allowedFreeMoveArea->size.y - radius);
				GetNode()->SetLocalPosition(newPos);
				_dragStartPosition = pointerWorldPos;
			}
		}
		else if (auto e = event.getIf<sf::Event::MouseButtonReleased>()) {
			_dragStartPosition.reset();
		}
	}

	void BilliardBallBehaviour::SetAllowedFreeMoveArea(const sf::FloatRect& allowedFreeMoveArea) {
		_allowedFreeMoveArea = allowedFreeMoveArea;
	}

	void BilliardBallBehaviour::ResetAllowedFreeMoveArea() {
		_allowedFreeMoveArea.reset();
	}
} // namespace Billiard
