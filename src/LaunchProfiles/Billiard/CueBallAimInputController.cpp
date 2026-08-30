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

	void CueBallAimInputController::OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button) {
		if (!_inputEnabled || button != sf::Mouse::Button::Left) {
			return;
		}
		auto aimWidget = _aimWidget.lock();
		if (!aimWidget) {
			return;
		}

		const sf::Vector2f worldPoint = MapPixelToWorld(position);
		if (!aimWidget->TrySetAimPointFromWorld(worldPoint)) {
			return;
		}
		_isPointerDown = true;
	}

	void CueBallAimInputController::OnMouseMoved(const sf::Vector2i& position) {
		if (!_inputEnabled || !_isPointerDown) {
			return;
		}
		if (auto aimWidget = _aimWidget.lock()) {
			aimWidget->TrySetAimPointFromWorld(MapPixelToWorld(position));
		}
	}

	void CueBallAimInputController::OnMouseButtonReleased(const sf::Vector2i& /*position*/, sf::Mouse::Button button) {
		if (button == sf::Mouse::Button::Left) {
			_isPointerDown = false;
		}
	}

} // namespace Billiard
