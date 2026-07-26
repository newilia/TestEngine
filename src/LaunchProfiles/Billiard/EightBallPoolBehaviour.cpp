#include "EightBallPoolBehaviour.h"

#include "EightBallPoolBehaviour.generated.hpp"

namespace Billiard {

	void EightBallPoolBehaviour::OnInit() {
		WirePocketSignals();

		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			_aimPointChangedSubscription =
			    aimDisplay->GetAimPointChangedSignal().Subscribe([this](const sf::Vector2f& aimPoint) {
				    OnAimPointChanged(aimPoint);
			    });
		}
	}

	void EightBallPoolBehaviour::OnDeinit() {
		UnsubscribeAll();
	}

	void EightBallPoolBehaviour::StartNewGame() {
		_phase = GamePhase::Break;
		_activePlayerIndex = 0;

		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetPlayerScore(0, 0);
			scoreboard->SetPlayerScore(1, 0);
			scoreboard->SetActivePlayer(_activePlayerIndex);
			scoreboard->ShowMessage("");
		}

		for (auto& pocket : _pocketsBehaviours) {
			if (auto pocketBehaviour = pocket.Get()) {
				pocketBehaviour->Reset();
			}
		}

		if (auto ballSpawn = _ballSpawnBehaviour.Get()) {
			auto spawnedBalls = ballSpawn->SpawnBalls();

			for (auto& ball : spawnedBalls) {
				if (auto ballBehaviour = ball->FindBehaviour<BilliardBallBehaviour>()) {
					// todo make RefWrapper getter and constructor
					auto ballRef = RefWrapper<BilliardBallBehaviour>();
					ballRef.SetId(ballBehaviour->GetEntityId());
					_ballsBehaviours[ballBehaviour->GetBallNumber()] = ballRef;

					for (auto& pocket : _pocketsBehaviours) {
						if (auto pocketBehaviour = pocket.Get()) {
							pocketBehaviour->RegisterBall(*ballBehaviour);
						}
					}
				}
			}
		}

		SpawnCue();
	}

	void EightBallPoolBehaviour::SpawnCue() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			if (auto node = cueBehaviour->GetNode()) {
				node->RemoveFromParent();
			}
			_cueBehaviour.Clear();
		}

		if (auto cueAsset = _cueAsset.Get()) {
			if (auto cueParent = _cueParent.Get()) {
				if (auto cueNode = cueAsset->InstantiateOn(cueParent)) {
					if (auto cueBehaviour = cueNode->FindBehaviour<BilliardCueBehaviour>()) {
						RefWrapper<BilliardCueBehaviour> cueRef;
						cueRef.SetId(cueBehaviour->GetEntityId());
						_cueBehaviour = cueRef;
						SetCueOnBall(0);
					}
				}
			}
		}
	}

	void EightBallPoolBehaviour::SetCueOnBall(int ballNumber) {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			if (auto ball = _ballsBehaviours[ballNumber].Get()) {
				if (auto ballNode = ball->GetNode()) {
					cueBehaviour->SetTargetNode(ballNode);
					cueBehaviour->SetBallRadius(ball->GetRadius());
					cueBehaviour->SetDistanceFromTarget(50);
				}
			}
		}
	}

	void EightBallPoolBehaviour::WirePocketSignals() {
		for (auto& pocketRef : _pocketsBehaviours) {
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
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetActivePlayer(_activePlayerIndex);
		}
	}

	void EightBallPoolBehaviour::EndTurn() {
		StartTurn(1 - _activePlayerIndex);
	}

	void EightBallPoolBehaviour::OnAimPointChanged(const sf::Vector2f& aimPoint) {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->SetLateralPosition(aimPoint.x);
			cueBehaviour->SetVerticalSpin(aimPoint.y);
		}
	}

} // namespace Billiard
