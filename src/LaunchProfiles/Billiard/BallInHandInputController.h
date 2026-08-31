#pragma once

#include "BilliardBallBehaviour.h"
#include "BilliardTablePresenter.h"
#include "Engine/Core/Signal.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Mouse.hpp>

#include <memory>
#include <optional>

namespace Billiard {

	class BallInHandInputController
	{
	public:
		explicit BallInHandInputController(BilliardTablePresenter& tablePresenter);

		void SetCueBall(std::weak_ptr<BilliardBallBehaviour> cueBall);
		void SetBallInHandRect(const sf::FloatRect& allowedMoveArea);
		void ResetBallInHand();
		[[nodiscard]] bool IsBallInHand() const;
		void SetInputEnabled(bool enabled);
		[[nodiscard]] bool IsInputEnabled() const;
		void OnMouseButtonPressed(const sf::Vector2i& position, sf::Mouse::Button button);
		void OnMouseMoved(const sf::Vector2i& position);
		void OnMouseButtonReleased(const sf::Vector2i& position);
		[[nodiscard]] bool IsDragging() const;
		Signal<>& GetOnGrabSignal() const;
		Signal<>& GetOnReleaseSignal() const;

	private:
		[[nodiscard]] sf::Vector2f MapPixelToWorld(sf::Vector2i pixel) const;
		void ApplyDragPosition(const sf::Vector2f& pointerWorldPos);
		void TryReleaseBallInHand();

		BilliardTablePresenter& _tablePresenter;
		std::weak_ptr<BilliardBallBehaviour> _cueBall;
		std::optional<sf::FloatRect> _allowedMoveArea;
		bool _inputEnabled = false;
		bool _isDragging = false;
		mutable Signal<> _onGrabSignal;
		mutable Signal<> _onReleaseSignal;
	};

} // namespace Billiard
