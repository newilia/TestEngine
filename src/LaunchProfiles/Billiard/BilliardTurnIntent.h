#pragma once

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <optional>

namespace Billiard {

	enum class TurnIntentPhase
	{
		Aim,
		BallInHand,
		Shoot,
	};

	struct TurnIntent
	{
		TurnIntentPhase phase = TurnIntentPhase::Aim;
		int playerIndex = 0;
		std::uint32_t turnId = 0;
		sf::Angle directionAngle{};
		float pullDistance = 0.f;
		float lateralSpin = 0.f;
		float verticalSpin = 0.f;
		std::optional<sf::Vector2f> cueBallPosition;
	};

} // namespace Billiard
