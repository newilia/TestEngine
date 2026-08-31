#include "CueInputController.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <cmath>
#include <limits>

namespace Billiard {

	namespace {
		constexpr float kTapMoveThresholdPixels = 2.f;
		constexpr float kWheelRotateRadiansPerNotch = -0.001f;
	} // namespace

	void CueInputController::SetCue(std::weak_ptr<BilliardCueBehaviour> cue) {
		_cue = std::move(cue);
	}

	void CueInputController::SetTableRect(RefWrapper<RectangleShapeVisual> tableRect) {
		_tableRect = std::move(tableRect);
	}

	void CueInputController::SetInputEnabled(bool enabled) {
		_inputEnabled = enabled;
		if (!enabled) {
			_isLeftButtonHeldOnTable = false;
			_leftButtonDragStarted = false;
			_hasPointerAngle = false;
			_isPullingBack = false;
			_pendingIntent.reset();
			if (auto cue = _cue.lock()) {
				cue->AbortAiming();
			}
		}
	}

	bool CueInputController::IsInputEnabled() const {
		return _inputEnabled;
	}

	bool CueInputController::HasPendingIntent() const {
		return _pendingIntent.has_value();
	}

	std::optional<TurnIntent> CueInputController::ConsumePendingIntent() {
		auto intent = _pendingIntent;
		_pendingIntent.reset();
		return intent;
	}

	sf::Vector2f CueInputController::MapPixelToWorld(sf::Vector2i pixel) const {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return {};
		}
		return Utils::MapWindowPixelToWorld(*window, pixel);
	}

	bool CueInputController::IsPointOnTable(const sf::Vector2f& worldPoint) const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->HitTest(worldPoint);
		}
		return false;
	}

	void CueInputController::RotateCueBy(float radiansDelta) {
		auto cue = _cue.lock();
		if (!cue || !cue->IsInteractable() || radiansDelta == 0.f) {
			return;
		}
		cue->SetDirectionAngle(cue->GetActualBallDirectionAngle() + sf::radians(radiansDelta));
	}

	float CueInputController::ShortestAngleDelta(float fromRadians, float toRadians) const {
		return std::atan2(std::sin(toRadians - fromRadians), std::cos(toRadians - fromRadians));
	}

	void CueInputController::UpdateDragRotation(const BilliardCueBehaviour& cue, const sf::Vector2f& worldPoint) {
		const auto ballPosition = cue.GetTargetBallWorldPosition();
		if (!ballPosition) {
			return;
		}

		const sf::Vector2f offset = worldPoint - *ballPosition;
		if (offset.lengthSquared() <= std::numeric_limits<float>::epsilon()) {
			return;
		}

		const float pointerAngle = std::atan2(offset.y, offset.x);
		if (_hasPointerAngle) {
			RotateCueBy(ShortestAngleDelta(_lastPointerAngleRadians, pointerAngle));
		}
		_lastPointerAngleRadians = pointerAngle;
		_hasPointerAngle = true;
	}

	void CueInputController::UpdatePullBack(const sf::Vector2f& worldPoint) {
		auto cue = _cue.lock();
		if (!cue || !_isPullingBack) {
			return;
		}

		const float angle = cue->GetActualBallDirectionAngle().asRadians();
		const sf::Vector2f cueDir(std::cos(angle), std::sin(angle));
		const sf::Vector2f pointerDelta = worldPoint - _pullBackGrabWorldPoint;
		cue->SetDistanceFromTarget(_pullBackDistanceAtGrab - Utils::Dot(pointerDelta, cueDir));
	}

	bool CueInputController::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button) {
		if (!_inputEnabled) {
			return false;
		}
		auto cue = _cue.lock();
		if (!cue || !cue->IsInteractable()) {
			return false;
		}

		const sf::Vector2f worldPoint = MapPixelToWorld(position);
		if (button == sf::Mouse::Button::Left) {
			if (!IsPointOnTable(worldPoint)) {
				return false;
			}
			_isLeftButtonHeldOnTable = true;
			_leftButtonDragStarted = false;
			_hasPointerAngle = false;
			_leftButtonPressPixel = position;
			return true;
		}
		if (button == sf::Mouse::Button::Right) {
			_isPullingBack = true;
			_pullBackGrabWorldPoint = worldPoint;
			_pullBackDistanceAtGrab = cue->GetDistanceFromTarget();
			return true;
		}

		return false;
	}

	bool CueInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled) {
			return false;
		}
		auto cue = _cue.lock();
		if (!cue) {
			return false;
		}

		bool handled = false;
		if (_isLeftButtonHeldOnTable) {
			const sf::Vector2f pixelDelta = sf::Vector2f(position - _leftButtonPressPixel);
			if (!_leftButtonDragStarted &&
			    pixelDelta.lengthSquared() > kTapMoveThresholdPixels * kTapMoveThresholdPixels) {
				_leftButtonDragStarted = true;
			}
			if (_leftButtonDragStarted) {
				UpdateDragRotation(*cue, MapPixelToWorld(position));
			}
			handled = true;
		}

		if (_isPullingBack) {
			UpdatePullBack(MapPixelToWorld(position));
			handled = true;
		}
		return handled;
	}

	bool CueInputController::OnMouseWheelScrolled(float wheelDelta) {
		if (!_inputEnabled || wheelDelta == 0.f) {
			return false;
		}
		auto cue = _cue.lock();
		if (!cue || !cue->IsInteractable()) {
			return false;
		}
		RotateCueBy(wheelDelta * kWheelRotateRadiansPerNotch);
		return true;
	}

	bool CueInputController::OnMouseButtonReleased(
	    const sf::Vector2i& position, sf::Mouse::Button button, int playerIndex, std::uint32_t turnId) {
		if (!_inputEnabled) {
			return false;
		}
		auto cue = _cue.lock();
		if (!cue) {
			return false;
		}

		if (button == sf::Mouse::Button::Left) {
			if (!_isLeftButtonHeldOnTable) {
				return false;
			}
			if (!_leftButtonDragStarted && cue->IsInteractable()) {
				const sf::Vector2f worldPoint = MapPixelToWorld(position);
				if (IsPointOnTable(worldPoint)) {
					cue->AimAt(worldPoint);
				}
			}
			_isLeftButtonHeldOnTable = false;
			_leftButtonDragStarted = false;
			_hasPointerAngle = false;
			return true;
		}
		else if (button == sf::Mouse::Button::Right) {
			if (!_isPullingBack) {
				return false;
			}

			if (cue->IsInteractable()) {
				_pendingIntent = cue->BuildTurnIntent(playerIndex, turnId);
			}
			cue->Release();
			_isPullingBack = false;
			return true;
		}

		return false;
	}

} // namespace Billiard
