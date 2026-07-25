#include "EightBallPoolBehaviour.h"

#include "EightBallPoolBehaviour.generated.hpp"

namespace Billiard {

	void EightBallPoolBehaviour::OnInit() {
		WirePocketSignals();
	}

	void EightBallPoolBehaviour::OnDeinit() {
		UnsubscribeAll();
	}

	void EightBallPoolBehaviour::StartNewGame() {
		_phase = GamePhase::Break;
		_activePlayerIndex = 0;

		if (auto scoreboard = _scoreboard.Get()) {
			scoreboard->SetPlayerScore(0, 0);
			scoreboard->SetPlayerScore(1, 0);
			scoreboard->SetActivePlayer(_activePlayerIndex);
			scoreboard->ShowMessage("");
		}

		if (auto ballSpawn = _ballSpawn.Get()) {
			auto spawnedBalls = ballSpawn->SpawnBalls();

			for (auto& ball : spawnedBalls) {
				if (auto ballBehaviour = ball->FindBehaviour<BilliardBallBehaviour>()) {
					// todo make RefWrapper getter and constructor
					auto ballRef = RefWrapper<BilliardBallBehaviour>();
					ballRef.SetId(ball->GetEntityId());
					_balls.push_back(ballRef);

					for (auto& pocket : _pockets) {
						if (auto pocketBehaviour = pocket.Get()) {
							pocketBehaviour->RegisterBall(*ballBehaviour);
						}
					}
				}
			}
		}
	}

	void EightBallPoolBehaviour::WirePocketSignals() {
		for (auto& pocketRef : _pockets) {
			if (auto pocket = pocketRef.Get()) {
				Subscribe(pocket->GetOnBallFallSignal(), [this](int ballNumber) {
					OnBallFellInPocket(ballNumber);
				});
			}
		}
	}

	void EightBallPoolBehaviour::OnBallFellInPocket(int /*ballNumber*/) {
		// 8-ball rules: fouls, scoring, and turn changes will be implemented here.
	}

	void EightBallPoolBehaviour::StartTurn(int playerIndex) {
		_activePlayerIndex = playerIndex;
		if (auto scoreboard = _scoreboard.Get()) {
			scoreboard->SetActivePlayer(_activePlayerIndex);
		}
	}

	void EightBallPoolBehaviour::EndTurn() {
		StartTurn(1 - _activePlayerIndex);
	}

} // namespace Billiard
