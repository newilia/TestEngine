#include "EightBallPoolGame.h"

namespace Billiard {

	namespace {
		BallType GetBallType(int ballNumber) {
			if (ballNumber == 0) {
				return BallType::Cue;
			}
			if (ballNumber == 8) {
				return BallType::Eight;
			}
			if (ballNumber >= 1 && ballNumber <= 7) {
				return BallType::Striped;
			}
			if (ballNumber >= 9 && ballNumber <= 15) {
				return BallType::Solid;
			}
		}
	} // namespace

	void EightBallPoolGame::StartNewGame() {
		_phase = GamePhase::Aiming;
		_activePlayerIndex = 0;
		_isBreakShot = true;
		_isFoul = false;
		_isBallInHand = false;
		_isCueBallPocketed = false;
		_isEightBallPocketed = false;
		_isCueBallCollideBall = false;
		_pocketedSolids.clear();
		_pocketedStripes.clear();
		_playerBallTypes = {BallType::Undefined, BallType::Undefined};
		_winnerIndex = std::nullopt;
		_pocketedBallsOnCurrentTurn.clear();
		_ballsHitRailOnCurrentTurn.clear();
	}

	void EightBallPoolGame::OnShoot() {
		assert(_phase == GamePhase::Aiming);
		_phase = GamePhase::WaitingForBallsToStop;
	}

	void EightBallPoolGame::OnBallFellInPocket(int ballNumber) {
		assert(_phase == GamePhase::WaitingForBallsToStop);

		auto ballType = GetBallType(ballNumber);
		if (ballType == BallType::Cue) {
			_isCueBallPocketed = true;
			_isFoul = true;
		}
		else if (ballType == BallType::Eight) {
			_isEightBallPocketed = true;
		}
		else if (ballType == BallType::Solid) {
			_pocketedSolids.insert(ballNumber);
		}
		else if (ballType == BallType::Striped) {
			_pocketedStripes.insert(ballNumber);
		}

		_pocketedBallsOnCurrentTurn.push_back(ballNumber);
	}

	void EightBallPoolGame::OnBallRemovedFromPocket(int ballNumber) {
		auto ballType = GetBallType(ballNumber);
		if (ballType == BallType::Cue) {
			_isCueBallPocketed = false;
		}
		else if (ballType == BallType::Eight) {
			_isEightBallPocketed = false;
		}
		else if (ballType == BallType::Solid) {
			_pocketedSolids.erase(ballNumber);
		}
		else if (ballType == BallType::Striped) {
			_pocketedStripes.erase(ballNumber);
		}
	}

	void EightBallPoolGame::OnBallCollideRail(int ballNumber) {
		_ballsHitRailOnCurrentTurn.insert(ballNumber);
	}

	void EightBallPoolGame::OnCueBallCollideBall(int ballNumber) {
		if (_isCueBallCollideBall) {
			return;
		}
		_isCueBallCollideBall = true;
		auto ballType = GetBallType(ballNumber);
		auto playerBallType = _playerBallTypes[_activePlayerIndex];
		if (playerBallType != BallType::Undefined && playerBallType != ballType || ballType == BallType::Eight) {
			_isFoul = true;
		}
	}

	void EightBallPoolGame::OnBallsStopped() {
		assert(_phase == GamePhase::WaitingForBallsToStop);

		if (_isEightBallPocketed && !_isBreakShot) {
			_phase = GamePhase::GameOver;
			if (MayPocketEightBall() && !_isCueBallPocketed) {
				_winnerIndex = _activePlayerIndex;
			}
			else {
				_winnerIndex = 1 - _activePlayerIndex;
			}
			return;
		}

		if (_isBreakShot) {
			if (_ballsHitRailOnCurrentTurn.size() < 4) {
				_isFoul = true;
			}
			_isBreakShot = false;
		}
		else {
			if (_ballsHitRailOnCurrentTurn.empty()) {
				_isFoul = true;
			}
		}

		/* try assign ball types */
		if (_playerBallTypes[_activePlayerIndex] == BallType::Undefined && !_isFoul) {
			if (!_pocketedBallsOnCurrentTurn.empty()) {
				auto ballType = GetBallType(_pocketedBallsOnCurrentTurn.front());
				_playerBallTypes[_activePlayerIndex] = ballType;
				_playerBallTypes[1 - _activePlayerIndex] =
				    ballType == BallType::Striped ? BallType::Solid : BallType::Striped;
			}
		}
		_isBreakShot = false;

		if (_isFoul) {
			_activePlayerIndex = 1 - _activePlayerIndex;
			_isBallInHand = true;
		}
		else {
			_isBallInHand = false;
		}
	}

	bool EightBallPoolGame::MayPocketEightBall() const {
		auto ballType = _playerBallTypes[_activePlayerIndex];
		if (ballType == BallType::Solid) {
			return _pocketedSolids.size() == 7;
		}
		if (ballType == BallType::Striped) {
			return _pocketedStripes.size() == 7;
		}
		return false;
	}

	bool EightBallPoolGame::IsBallInHand() const {
		return _isBallInHand;
	}

	bool EightBallPoolGame::IsBallInKitchen() const {
		return _isBreakShot;
	}

	const std::set<int>& EightBallPoolGame::GetPocketedSolids() const {
		return _pocketedSolids;
	}

	const std::set<int>& EightBallPoolGame::GetPocketedStripes() const {
		return _pocketedStripes;
	}

	bool EightBallPoolGame::IsCueBallPocketed() const {
		return _isCueBallPocketed;
	}

	bool EightBallPoolGame::IsEightBallPocketed() const {
		return _isEightBallPocketed;
	}

	int EightBallPoolGame::GetActivePlayerIndex() const {
		return _activePlayerIndex;
	}

	bool EightBallPoolGame::IsGameOver() const {
		return _phase == GamePhase::GameOver;
	}

	int EightBallPoolGame::GetWinnerIndex() const {
		return _winnerIndex.value_or(-1);
	}

	void EightBallPoolGame::StartNewTurn() {
		_phase = GamePhase::Aiming;
		_isFoul = false;
		_pocketedBallsOnCurrentTurn.clear();
		_ballsHitRailOnCurrentTurn.clear();
		_isCueBallCollideBall = false;
	}

} // namespace Billiard
