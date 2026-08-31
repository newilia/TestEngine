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
		return _isDragging;
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
					_isDragging = true;
					if (auto physicsBody = cueBall->GetPhysicsBody()) {
						physicsBody->GetOverlapGroups() = {};
						physicsBody->GetCollisionGroups() = {};
					}
					_onGrabSignal.Emit();
					ApplyDragPosition(worldPos);
				}
			}
		}
	}

	void BallInHandInputController::ApplyDragPosition(const sf::Vector2f& pointerWorldPos) {
		auto cueBall = _cueBall.lock();
		if (!cueBall || !IsBallInHand() || !_isDragging || !_allowedMoveArea) {
			return;
		}

		auto node = cueBall->GetNode();
		if (!node) {
			return;
		}

		sf::Vector2f localPointer = pointerWorldPos;
		if (auto parent = node->GetParent()) {
			localPointer = parent->GetWorldTransform().getInverse().transformPoint(pointerWorldPos);
		}

		localPointer.x = std::clamp(
		    localPointer.x, _allowedMoveArea->position.x, _allowedMoveArea->position.x + _allowedMoveArea->size.x);
		localPointer.y = std::clamp(
		    localPointer.y, _allowedMoveArea->position.y, _allowedMoveArea->position.y + _allowedMoveArea->size.y);

		if (const auto newPos = _tablePresenter.GetNearestFreeBallPosition(localPointer, 0)) {
			node->SetLocalPosition(*newPos);
		}
	}

	void BallInHandInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled || !_isDragging) {
			return;
		}
		ApplyDragPosition(MapPixelToWorld(position));
	}

	void BallInHandInputController::OnMouseButtonReleased(const sf::Vector2i& /*position*/) {
		TryReleaseBallInHand();
	}

	void BallInHandInputController::TryReleaseBallInHand() {
		if (!_isDragging) {
			return;
		}
		_isDragging = false;
		if (auto cueBall = _cueBall.lock()) {
			cueBall->RestoreCollisionGroups();
		}
		_onReleaseSignal.Emit(); // todo check
	}

} // namespace Billiard
