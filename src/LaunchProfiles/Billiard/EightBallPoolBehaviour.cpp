#include "EightBallPoolBehaviour.h"

#include "EightBallPoolBehaviour.generated.hpp"

namespace Billiard {

	void EightBallPoolBehaviour::OnInit() {
		for (auto& pocketRef : _pocketsBehaviours) {
			if (auto pocket = pocketRef.Get()) {
				Subscribe(pocket->GetOnBallFallSignal(), [this](int ballNumber) {
					OnBallFellInPocket(ballNumber);
				});
			}
		}

		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			Subscribe(aimDisplay->GetAimPointChangedSignal(), [this](const sf::Vector2f& aimPoint) {
				OnAimPointChanged(aimPoint);
			});
		}

		if (auto cueBehaviour = _cueBehaviour.Get()) {
			Subscribe(cueBehaviour->GetOnHitSignal(), [this]() {
				OnCueHit();
			});
		}
	}

	void EightBallPoolBehaviour::OnDeinit() {
		UnsubscribeAll();
	}

	void EightBallPoolBehaviour::OnUpdate(const sf::Time& deltaTime) {
		if (_isWaitingForBallsToStop) {
			if (!AreBallsMoving()) {
				OnBallsStopped();
			}
		}
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
					_ballsBehaviours[ballBehaviour->GetBallNumber()] = ballBehaviour;

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
						_cueBehaviour = cueBehaviour;
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

	void EightBallPoolBehaviour::OnBallFellInPocket(int ballNumber) {
		// 8-ball rules: fouls, scoring, and turn changes will be implemented here.
		_pocketedBalls.insert(ballNumber);
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

	void EightBallPoolBehaviour::OnCueHit() {
		_isWaitingForBallsToStop = true;
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
	}

	void EightBallPoolBehaviour::OnBallsStopped() {
		_isWaitingForBallsToStop = false;
		if (_pocketedBalls.contains(0)) {
			RestoreCueBall();
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
		}
		SetCueOnBall(0);
	}

	bool EightBallPoolBehaviour::AreBallsMoving() const {
		for (const auto& [_, ball] : _ballsBehaviours) {
			if (auto ballBehaviour = ball.Get()) {
				if (!ballBehaviour->GetNode()->IsEnabled()) {
					continue;
				}
				if (auto physicsBody = ballBehaviour->GetPhysicsBody()) {
					if (physicsBody->GetVelocity().length() > 0.1f || physicsBody->GetAngularSpeed() > 0.01f) {
						return true;
					}
				}
			}
		}
		return false;
	}

	void EightBallPoolBehaviour::RestoreCueBall() {
		if (auto cueBall = _ballsBehaviours[0].Get()) {
			cueBall->Appear();
			cueBall->GetNode()->SetLocalPosition(sf::Vector2f(0, 0));
			if (auto physicsBody = cueBall->GetPhysicsBody()) {
				physicsBody->SetVelocity(sf::Vector2f(0, 0));
				physicsBody->SetAngularSpeed(0.f);
				physicsBody->GetCollisionGroups().reset();
				physicsBody->GetCollisionGroups().set(0, true);
			}
			_pocketedBalls.erase(0);
		}
	}

} // namespace Billiard
