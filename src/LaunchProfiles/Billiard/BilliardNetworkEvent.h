#pragma once

#include "BilliardRulesSnapshot.h"
#include "BilliardTableSnapshot.h"

#include <SFML/System/Angle.hpp>

#include <cstdint>
#include <deque>

namespace Billiard {

	enum class BilliardNetworkEventType
	{
		GameStarted,
		CueAimUpdate,
		TableStateUpdate,
		TurnResult,
		BallInHandDragStarted,
		BallInHandDragEnded,
		CueReleased,
	};

	struct BilliardNetworkEvent
	{
		BilliardNetworkEventType type = BilliardNetworkEventType::GameStarted;
		int playerIndex = 0;
		std::uint32_t turnId = 0;
		int nextActivePlayer = 0;
		TableSnapshot table;
		RulesSnapshot rules;
		sf::Angle directionAngle{};
		float pullDistance = 0.f;
		float lateralSpin = 0.f;
		float verticalSpin = 0.f;
	};

	using BilliardNetworkEventQueue = std::deque<BilliardNetworkEvent>;

} // namespace Billiard
