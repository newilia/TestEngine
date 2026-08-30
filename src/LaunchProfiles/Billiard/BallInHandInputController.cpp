#include "BallInHandInputController.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Visual/CircleShapeVisual.h"

#include <algorithm>

namespace Billiard {

	BallInHandInputController::BallInHandInputController(BilliardTablePresenter& tablePresenter)
	    : _tablePresenter(tablePresenter) {}

	void BallInHandInputController::SetCueBall(std::weak_ptr<BilliardBallBehaviour> cueBall) {
		_cueBall = std::move(cueBall);
	}

	void BallInHandInputController::SetBallInHandRect(const sf::FloatRect& allowedMoveArea) {
		_allowedMoveArea = allowedMoveArea;
	}

	void BallInHandInputController::ResetBallInHand() {
		_allowedMoveArea.reset();
	}

	bool BallInHandInputController::IsBallInHand() const {
		return _allowedMoveArea.has_value();
	}

	void BallInHandInputController::SetInputEnabled(bool enabled) {
		_inputEnabled = enabled;
		if (!enabled) {
			TryReleaseBallInHand();
		}
	}

	bool BallInHandInputController::IsInputEnabled() const {
		return _inputEnabled;
	}

	bool BallInHandInputController::IsDragging() const {
		return _lastPointerWorld.has_value();
	}

	Signal<>& BallInHandInputController::GetOnGrabSignal() const {
		return _onGrabSignal;
	}

	Signal<>& BallInHandInputController::GetOnReleaseSignal() const {
		return _onReleaseSignal;
	}

	sf::Vector2f BallInHandInputController::MapPixelToWorld(sf::Vector2i pixel) const {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return {};
		}
		return Utils::MapWindowPixelToWorld(*window, pixel);
	}

	void BallInHandInputController::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button /*button*/) {
		if (!_inputEnabled || !IsBallInHand()) {
			return;
		}
		auto cueBall = _cueBall.lock();
		if (!cueBall) {
			return;
		}

		const sf::Vector2f worldPos = MapPixelToWorld(position);
		if (auto node = cueBall->GetNode()) {
			if (auto visual = node->GetVisual<CircleShapeVisual>()) {
				if (visual->HitTest(worldPos)) {
					_lastPointerWorld = worldPos;
					if (auto physicsBody = cueBall->GetPhysicsBody()) {
						physicsBody->GetOverlapGroups() = {};
						physicsBody->GetCollisionGroups() = {};
					}
					_onGrabSignal.Emit();
				}
			}
		}
	}

	void BallInHandInputController::ApplyDragPosition(const sf::Vector2f& pointerWorldPos) {
		auto cueBall = _cueBall.lock();
		if (!cueBall || !IsBallInHand() || !_lastPointerWorld || !_allowedMoveArea) {
			return;
		}

		auto node = cueBall->GetNode();
		if (!node) {
			return;
		}

		auto newPos = node->GetLocalPosition();
		const sf::Vector2f delta = pointerWorldPos - *_lastPointerWorld;
		newPos += delta;

		//const float radius = cueBall->GetRadius();
		newPos.x =
		    std::clamp(newPos.x, _allowedMoveArea->position.x, _allowedMoveArea->position.x + _allowedMoveArea->size.x);
		newPos.y =
		    std::clamp(newPos.y, _allowedMoveArea->position.y, _allowedMoveArea->position.y + _allowedMoveArea->size.y);

		newPos = _tablePresenter.GetNearestFreeBallPosition(newPos, 0);
		node->SetLocalPosition(newPos);
		_lastPointerWorld = pointerWorldPos;
	}

	void BallInHandInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled || !_lastPointerWorld) {
			return;
		}
		ApplyDragPosition(MapPixelToWorld(position));
	}

	void BallInHandInputController::OnMouseButtonReleased(const sf::Vector2i& /*position*/) {
		TryReleaseBallInHand();
	}

	void BallInHandInputController::TryReleaseBallInHand() {
		if (!_lastPointerWorld) {
			return;
		}
		_lastPointerWorld.reset(); // todo check
		if (auto cueBall = _cueBall.lock()) {
			cueBall->RestoreCollisionGroups();
		}
		_onReleaseSignal.Emit(); // todo check
	}

} // namespace Billiard
