#pragma once

#include "BilliardCueBehaviour.h"
#include "BilliardTurnIntent.h"

#include <SFML/Window/Mouse.hpp>

#include <cstdint>
#include <memory>
#include <optional>

namespace Billiard {

	class CueInputController
	{
	public:
		void SetCue(std::weak_ptr<BilliardCueBehaviour> cue);
		void SetInputEnabled(bool enabled);
		[[nodiscard]] bool IsInputEnabled() const;
		void OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button);
		void OnMouseMoved(const sf::Vector2i& position);
		void OnMouseButtonReleased(
		    const sf::Vector2i& position, sf::Mouse::Button button, int playerIndex, std::uint32_t turnId);
		[[nodiscard]] bool HasPendingIntent() const;
		std::optional<TurnIntent> ConsumePendingIntent();

	private:
		[[nodiscard]] sf::Vector2f MapPixelToWorld(sf::Vector2i pixel) const;
		void UpdatePullBack(const sf::Vector2f& worldPoint);

		std::weak_ptr<BilliardCueBehaviour> _cue;
		bool _inputEnabled = false;
		bool _isAiming = false;
		bool _isPullingBack = false;
		sf::Vector2f _pullBackGrabWorldPoint{};
		float _pullBackDistanceAtGrab = 0.f;
		std::optional<TurnIntent> _pendingIntent;
	};

} // namespace Billiard
