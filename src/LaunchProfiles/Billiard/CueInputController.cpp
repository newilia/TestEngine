#include "CueInputController.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SfmlWindowUtils.h"

#include <cmath>

namespace Billiard {

	void CueInputController::SetCue(std::weak_ptr<BilliardCueBehaviour> cue) {
		_cue = std::move(cue);
	}

	void CueInputController::SetInputEnabled(bool enabled) {
		_inputEnabled = enabled;
		if (!enabled) {
			_isAiming = false;
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

	void CueInputController::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button) {
		if (!_inputEnabled) {
			return;
		}
		auto cue = _cue.lock();
		if (!cue || !cue->IsInteractable()) {
			return;
		}

		const sf::Vector2f worldPoint = MapPixelToWorld(position);
		if (!cue->HitTestWorld(worldPoint)) {
			return;
		}

		if (button == sf::Mouse::Button::Left) {
			_isAiming = true;
		}
		else if (button == sf::Mouse::Button::Right) {
			_isPullingBack = true;
			_pullBackGrabWorldPoint = worldPoint;
			_pullBackDistanceAtGrab = cue->GetDistanceFromTarget();
		}
	}

	void CueInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled) {
			return;
		}
		auto cue = _cue.lock();
		if (!cue) {
			return;
		}

		const sf::Vector2f worldPoint = MapPixelToWorld(position);
		if (_isAiming) {
			cue->AimAt(worldPoint);
		}
		if (_isPullingBack) {
			UpdatePullBack(worldPoint);
		}
	}

	void CueInputController::OnMouseButtonReleased(
	    const sf::Vector2i& /*position*/, sf::Mouse::Button button, int playerIndex, std::uint32_t turnId) {
		if (!_inputEnabled) {
			return;
		}
		auto cue = _cue.lock();
		if (!cue) {
			return;
		}

		if (button == sf::Mouse::Button::Left) {
			_isAiming = false;
			return;
		}
		if (button != sf::Mouse::Button::Right) {
			return;
		}

		if (_isPullingBack) {
			if (cue->IsInteractable()) {
				_pendingIntent = cue->BuildTurnIntent(playerIndex, turnId);
			}
			cue->Release();
			_isPullingBack = false;
		}
	}

} // namespace Billiard
