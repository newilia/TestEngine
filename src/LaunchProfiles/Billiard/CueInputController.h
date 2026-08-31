#pragma once

#include "BilliardCueBehaviour.h"
#include "BilliardTurnIntent.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Window/Mouse.hpp>

#include <cstdint>
#include <memory>
#include <optional>

namespace Billiard {

	class CueInputController
	{
	public:
		void SetCue(std::weak_ptr<BilliardCueBehaviour> cue);
		void SetTableRect(RefWrapper<RectangleShapeVisual> tableRect);
		void SetInputEnabled(bool enabled);
		[[nodiscard]] bool IsInputEnabled() const;
		void OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button);
		void OnMouseMoved(const sf::Vector2i& position);
		void OnMouseWheelScrolled(float wheelDelta);
		void OnMouseButtonReleased(
		    const sf::Vector2i& position, sf::Mouse::Button button, int playerIndex, std::uint32_t turnId);
		[[nodiscard]] bool HasPendingIntent() const;
		std::optional<TurnIntent> ConsumePendingIntent();

	private:
		[[nodiscard]] sf::Vector2f MapPixelToWorld(sf::Vector2i pixel) const;
		[[nodiscard]] bool IsPointOnTable(const sf::Vector2f& worldPoint) const;
		[[nodiscard]] float ShortestAngleDelta(float fromRadians, float toRadians) const;
		void UpdatePullBack(const sf::Vector2f& worldPoint);
		void RotateCueBy(float radiansDelta);
		void UpdateDragRotation(const BilliardCueBehaviour& cue, const sf::Vector2f& worldPoint);

		std::weak_ptr<BilliardCueBehaviour> _cue;
		RefWrapper<RectangleShapeVisual> _tableRect;
		bool _inputEnabled = false;
		bool _isLeftButtonHeldOnTable = false;
		bool _leftButtonDragStarted = false;
		bool _hasPointerAngle = false;
		sf::Vector2i _leftButtonPressPixel{};
		float _lastPointerAngleRadians = 0.f;
		bool _isPullingBack = false;
		sf::Vector2f _pullBackGrabWorldPoint{};
		float _pullBackDistanceAtGrab = 0.f;
		std::optional<TurnIntent> _pendingIntent;
	};

} // namespace Billiard
