#include "BilliardBallBehaviour.h"

#include "BilliardBallBehaviour.generated.hpp"

#include <Engine/Core/SfmlWindowUtils.h>

namespace {
	sf::Vector2f toWorld(sf::Vector2i pixel) {
		return Utils::MapWindowPixelToWorld(*Engine::MainContext::GetInstance().GetMainWindow(), pixel);
	};
} // namespace

namespace Billiard {

	void BilliardBallBehaviour::SetInputEnabled(bool enabled) {
		_inputEnabled = enabled;
		if (!enabled) {
			_dragStartPosition.reset();
		}
	}

	bool BilliardBallBehaviour::IsInputEnabled() const {
		return _inputEnabled;
	}

	sf::Vector2f BilliardBallBehaviour::GetWorldPosition() const {
		if (auto node = GetNode()) {
			return node->GetLocalPosition();
		}
		return {};
	}

	void BilliardBallBehaviour::HandleMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button) {
		if (_inputEnabled) {
			OnMouseButtonPressed(position, button);
		}
	}

	void BilliardBallBehaviour::HandleMouseMoved(const sf::Vector2i& position) {
		if (_inputEnabled) {
			OnMouseMoved(position);
		}
	}

	void BilliardBallBehaviour::HandleMouseButtonReleased(const sf::Vector2i& position) {
		if (_inputEnabled) {
			OnMouseButtonReleased(position);
		}
	}

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

	void BilliardBallBehaviour::SetBallInHand(const sf::FloatRect& allowedMoveArea) {
		_ballInHandArea = allowedMoveArea;
	}

	void BilliardBallBehaviour::ResetBallInHand() {
		_ballInHandArea.reset();
	}

	bool BilliardBallBehaviour::IsBallInHand() const {
		return _ballInHandArea.has_value();
	}

	Signal<>& BilliardBallBehaviour::GetOnGrabSignal() const {
		return _onGrabSignal;
	}

	Signal<>& BilliardBallBehaviour::GetOnReleaseSignal() const {
		return _onReleaseSignal;
	}

	void BilliardBallBehaviour::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button /*button*/) {
		if (IsBallInHand()) {
			auto worldPos = toWorld(position);
			if (auto visual = _ballShape.Get()) {
				if (visual->HitTest(worldPos)) {
					_dragStartPosition = worldPos;
					if (auto physicsBody = _physicsBody.Get()) {
						_overlapGroupsBeforeGrab = physicsBody->GetOverlapGroups();
						_collisionGroupsBeforeGrab = physicsBody->GetCollisionGroups();
						physicsBody->GetOverlapGroups() = {};
						physicsBody->GetCollisionGroups() = {};
					}
					_onGrabSignal.Emit();
				}
			}
		}
	}

	void BilliardBallBehaviour::OnMouseMoved(const sf::Vector2i& position) {
		if (_dragStartPosition && IsBallInHand()) {
			auto newPos = GetNode()->GetLocalPosition();
			auto pointerWorldPos = toWorld(position);
			auto delta = pointerWorldPos - *_dragStartPosition;
			newPos += delta;
			auto radius = GetRadius();
			newPos.x = std::clamp(newPos.x, _ballInHandArea->position.x + radius,
			    _ballInHandArea->position.x + _ballInHandArea->size.x - radius);
			newPos.y = std::clamp(newPos.y, _ballInHandArea->position.y + radius,
			    _ballInHandArea->position.y + _ballInHandArea->size.y - radius);
			GetNode()->SetLocalPosition(newPos);
			_dragStartPosition = pointerWorldPos;
		}
	}

	void BilliardBallBehaviour::OnMouseButtonReleased(const sf::Vector2i& position) {
		if (_dragStartPosition) {
			_dragStartPosition.reset();
			if (auto physicsBody = _physicsBody.Get()) {
				physicsBody->GetOverlapGroups() = _overlapGroupsBeforeGrab;
				physicsBody->GetCollisionGroups() = _collisionGroupsBeforeGrab;
			}
			_onReleaseSignal.Emit();
			// TODO check if ball is not overlapping with any other ball
		}
	}
} // namespace Billiard
