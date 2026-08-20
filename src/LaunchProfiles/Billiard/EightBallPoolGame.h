#pragma once
#include "BallType.h"
#include "BilliardGamePhase.h"
#include "BilliardRulesSnapshot.h"

#include <array>
#include <optional>
#include <set>
#include <vector>

namespace Billiard {

	class EightBallPoolGame
	{
	public:
		void StartNewGame();
		void OnShoot();
		void OnBallFellInPocket(int ballNumber);
		void OnBallRemovedFromPocket(int ballNumber);
		void OnBallCollideRail(int ballNumber);
		void OnCueBallCollideBall(int ballNumber);
		void OnBallsStopped();
		void StartNewTurn();
		void OnTurnTimeOver();

		bool IsBallInHand() const;
		bool IsBallInKitchen() const;
		int GetActivePlayerIndex() const;
		BallType GetPlayerBallType(int playerIndex) const;
		const std::set<int>& GetPocketedSolids() const;
		const std::set<int>& GetPocketedStripes() const;
		bool IsCueBallPocketed() const;
		bool IsEightBallPocketed() const;
		bool IsGameOver() const;
		int GetWinnerIndex() const;
		[[nodiscard]] FoulKind GetFoulKind() const;
		[[nodiscard]] RulesSnapshot ToSnapshot() const;
		void ApplySnapshot(const RulesSnapshot& snapshot);

	private:
		bool MayPocketEightBall() const;

		GamePhase _phase = GamePhase::Aiming;
		int _activePlayerIndex = 0;
		bool _isBreakShot = true;
		std::set<int> _pocketedSolids;
		std::set<int> _pocketedStripes;
		std::vector<int> _pocketedBallsOnCurrentTurn;
		std::set<int> _ballsHitRailOnCurrentTurn;
		bool _isCueBallPocketed = false;
		bool _isEightBallPocketed = false;
		FoulKind _foulKind = FoulKind::None;
		bool _isBallInHand = false;
		std::array<BallType, 2> _playerBallTypes = {BallType::Undefined, BallType::Undefined};
		std::optional<int> _winnerIndex;
		bool _isCueBallCollideBall = false;
	};

} // namespace Billiard
