#include "CueBallAimInputController.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"

namespace Billiard {

	void CueBallAimInputController::SetAimWidget(std::weak_ptr<CueBallAimWidgetBehaviour> aimWidget) {
		_aimWidget = std::move(aimWidget);
	}

	void CueBallAimInputController::SetInputEnabled(bool enabled) {
		_inputEnabled = enabled;
		if (!enabled) {
			_isPointerDown = false;
		}
	}

	bool CueBallAimInputController::IsInputEnabled() const {
		return _inputEnabled;
	}

	sf::Vector2f CueBallAimInputController::MapPixelToWorld(sf::Vector2i pixel) const {
		auto window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return {};
		}
		return Utils::MapWindowPixelToWorld(*window, pixel);
	}

	bool CueBallAimInputController::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button) {
		if (!_inputEnabled || button != sf::Mouse::Button::Left) {
			return false;
		}
		auto aimWidget = _aimWidget.lock();
		if (!aimWidget) {
			return false;
		}

		const sf::Vector2f worldPoint = MapPixelToWorld(position);
		if (!aimWidget->TrySetAimPointFromWorld(worldPoint)) {
			return false;
		}
		_isPointerDown = true;
		return true;
	}

	bool CueBallAimInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled || !_isPointerDown) {
			return false;
		}
		if (auto aimWidget = _aimWidget.lock()) {
			aimWidget->TrySetAimPointFromWorld(MapPixelToWorld(position));
		}
		return true;
	}

	bool CueBallAimInputController::OnMouseButtonReleased(const sf::Vector2i& /*position*/, sf::Mouse::Button button) {
		if (button != sf::Mouse::Button::Left || !_isPointerDown) {
			return false;
		}
		_isPointerDown = false;
		return true;
	}

} // namespace Billiard
