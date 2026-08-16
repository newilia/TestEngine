#pragma once

#include "BallType.h"
#include "BilliardGamePhase.h"

#include <array>
#include <set>

namespace Billiard {

	struct RulesSnapshot
	{
		GamePhase phase = GamePhase::Aiming;
		int activePlayerIndex = 0;
		bool isBallInHand = false;
		bool isBreakShot = true;
		std::array<BallType, 2> playerBallTypes = {BallType::Undefined, BallType::Undefined};
		std::set<int> pocketedSolids;
		std::set<int> pocketedStripes;
		bool isGameOver = false;
		int winnerIndex = -1;
	};

} // namespace Billiard
