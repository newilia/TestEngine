#pragma once

#include "CueBallAimWidgetBehaviour.h"

#include <SFML/Window/Mouse.hpp>

#include <memory>

namespace Billiard {

	class CueBallAimInputController
	{
	public:
		void SetAimWidget(std::weak_ptr<CueBallAimWidgetBehaviour> aimWidget);
		void SetInputEnabled(bool enabled);
		[[nodiscard]] bool IsInputEnabled() const;
		void OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button);
		void OnMouseMoved(const sf::Vector2i& position);
		void OnMouseButtonReleased(const sf::Vector2i& position, sf::Mouse::Button button);

	private:
		[[nodiscard]] sf::Vector2f MapPixelToWorld(sf::Vector2i pixel) const;

		std::weak_ptr<CueBallAimWidgetBehaviour> _aimWidget;
		bool _inputEnabled = false;
		bool _isPointerDown = false;
	};

} // namespace Billiard
