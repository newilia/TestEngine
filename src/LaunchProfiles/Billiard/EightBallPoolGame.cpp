#include "EightBallPoolGame.h"

#include "BilliardRulesSnapshot.h"

namespace Billiard {

	namespace {
		BallType GetBallType(int ballNumber) {
			if (ballNumber == 0) {
				return BallType::Cue;
			}
			if (ballNumber >= 1 && ballNumber <= 7) {
				return BallType::Solid;
			}
			if (ballNumber == 8) {
				return BallType::Eight;
			}
			if (ballNumber >= 9 && ballNumber <= 15) {
				return BallType::Striped;
			}
			return BallType::Undefined;
		}

	} // namespace

	void EightBallPoolGame::StartNewGame() {
		_phase = GamePhase::Aiming;
		_activePlayerIndex = 0;
		_isBreakShot = true;
		_foulKind = FoulKind::None;
		_isBallInHand = false;
		_isCueBallCollideBall = false;
		_pocketedBalls.clear();
		_playerBallTypes = {BallType::Undefined, BallType::Undefined};
		_winnerIndex = std::nullopt;
		_pocketedBallsOnCurrentTurn.clear();
		_ballsHitRailOnCurrentTurn.clear();
	}

	void EightBallPoolGame::OnShoot() {
		assert(_phase == GamePhase::Aiming);
		_foulKind = FoulKind::None;
		_phase = GamePhase::WaitingForBallsToStop;
	}

	void EightBallPoolGame::OnBallFellInPocket(int ballNumber) {
		assert(_phase == GamePhase::WaitingForBallsToStop);

		_pocketedBalls.insert(ballNumber);

		auto ballType = GetBallType(ballNumber);
		if (ballType == BallType::Cue) {
			_foulKind = FoulKind::CueBallPocketed;
		}

		_pocketedBallsOnCurrentTurn.push_back(ballNumber);
	}

	void EightBallPoolGame::OnBallRemovedFromPocket(int ballNumber) {
		_pocketedBalls.erase(ballNumber);
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
		if (playerBallType != BallType::Undefined && playerBallType != ballType) {
			_foulKind = FoulKind::WrongBallFirst;
		}
	}

	void EightBallPoolGame::OnBallsStopped() {
		assert(_phase == GamePhase::WaitingForBallsToStop);

		if (_pocketedBalls.contains(8) && !_isBreakShot) {
			_phase = GamePhase::GameOver;
			if (MayPocketEightBall() && !_pocketedBalls.contains(0)) {
				_winnerIndex = _activePlayerIndex;
			}
			else {
				_winnerIndex = 1 - _activePlayerIndex;
			}
			return;
		}

		if (_isBreakShot) {
			if (_foulKind == FoulKind::None && _ballsHitRailOnCurrentTurn.size() < 4) {
				_foulKind = FoulKind::BreakInsufficientRails;
			}
			_isBreakShot = false;
		}
		else {
			if (_foulKind == FoulKind::None && _ballsHitRailOnCurrentTurn.empty()) {
				_foulKind = FoulKind::NoRailContact;
			}

			/* try assign ball types */
			if (_playerBallTypes[_activePlayerIndex] == BallType::Undefined && _foulKind == FoulKind::None) {
				if (!_pocketedBallsOnCurrentTurn.empty()) {
					auto ballType = GetBallType(_pocketedBallsOnCurrentTurn.front());
					_playerBallTypes[_activePlayerIndex] = ballType;
					_playerBallTypes[1 - _activePlayerIndex] =
					    ballType == BallType::Striped ? BallType::Solid : BallType::Striped;
				}
			}
		}

		if (_foulKind != FoulKind::None) {
			_activePlayerIndex = 1 - _activePlayerIndex;
			_isBallInHand = true;
		}
		else {
			const auto playerBallType = _playerBallTypes[_activePlayerIndex];
			bool hasValidPocketedBalls = false;
			if (playerBallType == BallType::Undefined && !_pocketedBallsOnCurrentTurn.empty()) {
				hasValidPocketedBalls = true;
			}
			else {
				for (const auto& ballNumber : _pocketedBallsOnCurrentTurn) {
					auto ballType = GetBallType(ballNumber);
					if (ballType == playerBallType) {
						hasValidPocketedBalls = true;
						break;
					}
				}
			}

			if (!hasValidPocketedBalls) {
				_activePlayerIndex = 1 - _activePlayerIndex;
			}
			_isBallInHand = false;
		}
	}

	bool EightBallPoolGame::MayPocketEightBall() const {
		auto ballType = _playerBallTypes[_activePlayerIndex];
		if (ballType == BallType::Solid) {
			return GetPocketedSolids().size() == 7;
		}
		if (ballType == BallType::Striped) {
			return GetPocketedStripes().size() == 7;
		}
		return false;
	}

	bool EightBallPoolGame::IsBallPocketed(int ballNumber) const {
		return _pocketedBalls.contains(ballNumber);
	}

	bool EightBallPoolGame::IsBallInHand() const {
		return _isBallInHand;
	}

	bool EightBallPoolGame::IsBallInKitchen() const {
		return _isBreakShot;
	}

	const std::set<int>& EightBallPoolGame::GetPocketedBalls() const {
		return _pocketedBalls;
	}

	const std::set<int>& EightBallPoolGame::GetPocketedSolids() const {
		_pocketedSolidsView.clear();
		for (const auto ballNumber : _pocketedBalls) {
			if (GetBallType(ballNumber) == BallType::Solid) {
				_pocketedSolidsView.insert(ballNumber);
			}
		}
		return _pocketedSolidsView;
	}

	const std::set<int>& EightBallPoolGame::GetPocketedStripes() const {
		_pocketedStripesView.clear();
		for (const auto ballNumber : _pocketedBalls) {
			if (GetBallType(ballNumber) == BallType::Striped) {
				_pocketedStripesView.insert(ballNumber);
			}
		}
		return _pocketedStripesView;
	}

	bool EightBallPoolGame::IsCueBallPocketed() const {
		return _pocketedBalls.contains(0);
	}

	bool EightBallPoolGame::IsEightBallPocketed() const {
		return _pocketedBalls.contains(8);
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

	FoulKind EightBallPoolGame::GetFoulKind() const {
		return _foulKind;
	}

	void EightBallPoolGame::StartNewTurn() {
		if (_phase == GamePhase::GameOver) {
			return;
		}
		_phase = GamePhase::Aiming;
		_pocketedBallsOnCurrentTurn.clear();
		_ballsHitRailOnCurrentTurn.clear();
		_isCueBallCollideBall = false;
	}

	void EightBallPoolGame::OnTurnTimeOver() {
		if (_phase != GamePhase::Aiming) {
			return;
		}
		_foulKind = FoulKind::TurnTimeOver;
		_isBallInHand = true;
		_activePlayerIndex = 1 - _activePlayerIndex;
	}

	BallType EightBallPoolGame::GetPlayerBallType(int playerIndex) const {
		return _playerBallTypes[playerIndex];
	}

	RulesSnapshot EightBallPoolGame::ToSnapshot() const {
		RulesSnapshot snapshot;
		snapshot.phase = _phase;
		snapshot.activePlayerIndex = _activePlayerIndex;
		snapshot.isBallInHand = _isBallInHand;
		snapshot.isBreakShot = _isBreakShot;
		snapshot.playerBallTypes = _playerBallTypes;
		snapshot.pocketedBalls = _pocketedBalls;
		snapshot.isGameOver = IsGameOver();
		snapshot.winnerIndex = _winnerIndex.value_or(-1);
		snapshot.foulKind = _foulKind;
		return snapshot;
	}

	void EightBallPoolGame::ApplySnapshot(const RulesSnapshot& snapshot) {
		_phase = snapshot.phase;
		_activePlayerIndex = snapshot.activePlayerIndex;
		_isBallInHand = snapshot.isBallInHand;
		_isBreakShot = snapshot.isBreakShot;
		_playerBallTypes = snapshot.playerBallTypes;
		_pocketedBalls = snapshot.pocketedBalls;
		_foulKind = snapshot.foulKind;
		_isCueBallCollideBall = false;
		_pocketedBallsOnCurrentTurn.clear();
		_ballsHitRailOnCurrentTurn.clear();
		if (snapshot.isGameOver) {
			_phase = GamePhase::GameOver;
			_winnerIndex = snapshot.winnerIndex >= 0 ? std::optional<int>(snapshot.winnerIndex) : std::nullopt;
		}
		else {
			_winnerIndex = std::nullopt;
		}
	}

} // namespace Billiard
