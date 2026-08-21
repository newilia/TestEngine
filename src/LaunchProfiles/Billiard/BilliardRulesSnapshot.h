#pragma once

#include "BallType.h"
#include "BilliardGamePhase.h"

#include <array>
#include <set>

namespace Billiard {

	enum class FoulKind
	{
		None,
		CueBallPocketed,
		WrongBallFirst,
		BreakInsufficientRails,
		NoRailContact,
		TurnTimeOver
	};

	struct RulesSnapshot
	{
		GamePhase phase = GamePhase::Aiming;
		int activePlayerIndex = 0;
		bool isBallInHand = false;
		bool isBreakShot = true;
		std::array<BallType, 2> playerBallTypes = {BallType::Undefined, BallType::Undefined};
		std::set<int> pocketedBalls;
		bool isGameOver = false;
		int winnerIndex = -1;
		FoulKind foulKind = FoulKind::None;
	};

} // namespace Billiard
