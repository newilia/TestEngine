#include "EightBallPoolBehaviour.h"

#include "EightBallPoolBehaviour.generated.hpp"
#include "Engine/Editor/Editor.h"
#include "RollingBallBehaviour.h"

namespace Billiard {

	void EightBallPoolBehaviour::OnInit() {
		InitSubscriptions();
	}

	void EightBallPoolBehaviour::OnDeinit() {
		UnsubscribeAll();
	}

	void EightBallPoolBehaviour::InitSubscriptions() {
		UnsubscribeAll();

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
			Subscribe(cueBehaviour->GetOnReleaseSignal(), [this]() {
				OnCueRelease();
			});
			Subscribe(cueBehaviour->GetOnHitSignal(), [this]() {
				OnCueHit();
			});
		}

		for (auto& [ballIndex, ballBehRef] : _ballsBehaviours) {
			if (auto ballBehaviour = ballBehRef.Get()) {
				if (auto physicsBody = ballBehaviour->GetPhysicsBody()) {
					Subscribe(
					    physicsBody->GetOnCollideSignal(), [this, weakBallBody = std::weak_ptr(physicsBody), ballIndex](
					                                           const IntersectionDetails& intersection) {
						    OnBallCollision(weakBallBody.lock(), ballIndex, intersection);
					    });
				}
				Subscribe(ballBehaviour->GetOnGrabSignal(), [this]() {
					OnBallInHandGrab();
				});
				Subscribe(ballBehaviour->GetOnReleaseSignal(), [this]() {
					OnBallInHandRelease();
				});
			}
		}
	}

	void EightBallPoolBehaviour::OnUpdate(const sf::Time& deltaTime) {
		if (_isWaitingForBallsToStop) {
			if (!AreBallsMoving()) {
				OnBallsStopped();
			}
		}
		else {
			if (_remainingTurnTime > 0.f) {
				_remainingTurnTime = std::max(0.f, _remainingTurnTime - deltaTime.asSeconds());
				if (_remainingTurnTime == 0.f) {
					_gameState.OnTurnTimeOver();
					StartNewTurn();
					UpdateScoreboard();
					if (auto cueBehaviour = _cueBehaviour.Get()) {
						cueBehaviour->AbortAiming();
					}
				}
				else {
					UpdateScoreboardTimer();
				}
			}
		}
	}

	void EightBallPoolBehaviour::StartNewGame() {
		Engine::Editor::GetInstance().SetCameraPanOnRightClickEnabled(false);

		_gameState.StartNewGame();

		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->Reset();
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

					if (ballBehaviour->GetBallNumber() == 0) {
						ballBehaviour->SetBallInHand(GetKitchenRect());
					}
				}
			}
		}

		SpawnCue();
		InitSubscriptions();
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
				cueBehaviour->SetTargetBall(ball);
			}
		}
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			aimDisplay->ResetAimPoint();
		}
	}

	void EightBallPoolBehaviour::OnBallFellInPocket(int ballNumber) {
		_gameState.OnBallFellInPocket(ballNumber);
	}

	void EightBallPoolBehaviour::OnAimPointChanged(const sf::Vector2f& aimPoint) {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->SetLateralPosition(aimPoint.x);
			cueBehaviour->SetVerticalSpin(aimPoint.y);
		}
	}

	void EightBallPoolBehaviour::OnCueRelease() {
		if (auto ball = _ballsBehaviours[0].Get()) {
			ball->ResetBallInHand();
		}
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		_gameState.OnShoot();
	}

	void EightBallPoolBehaviour::OnCueHit() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		_isWaitingForBallsToStop = true;
	}

	void EightBallPoolBehaviour::OnBallsStopped() {
		_isWaitingForBallsToStop = false;
		_gameState.OnBallsStopped();

		UpdateScoreboard();
		if (_gameState.IsGameOver()) {
			return;
		}

		if (_gameState.IsCueBallPocketed()) {
			RestoreBall(0);
		}
		if (_gameState.IsEightBallPocketed()) {
			RestoreBall(8); // TODO: restore 8 ball to proper position
		}

		if (_gameState.IsBallInHand()) {
			if (auto ball = _ballsBehaviours[0].Get()) {
				ball->SetBallInHand(GetBallInHandRect());
			}
		}

		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
		}
		SetCueOnBall(0);

		StartNewTurn();
	}

	void EightBallPoolBehaviour::StartNewTurn() {
		_remainingTurnTime = _turnTimeLimit;
		_gameState.StartNewTurn();
		UpdateScoreboard();
	}

	bool EightBallPoolBehaviour::AreBallsMoving() const {
		for (const auto& [_, ball] : _ballsBehaviours) {
			if (auto ballBehaviour = ball.Get()) {
				if (!ballBehaviour->GetNode()->IsEnabled()) {
					continue;
				}
				if (auto physicsBody = ballBehaviour->GetPhysicsBody()) {
					if (physicsBody->GetVelocity().length() > 1.f || std::abs(physicsBody->GetAngularSpeed()) > 0.1f) {
						return true;
					}
				}
			}
		}
		return false;
	}

	void EightBallPoolBehaviour::RestoreBall(int ballNumber) {
		if (auto ball = _ballsBehaviours[ballNumber].Get()) {
			ball->Appear();
			ball->GetNode()->SetLocalPosition(GetTableCenter()); // TODO: set to proper position for 8 ball

			if (auto physicsBody = ball->GetPhysicsBody()) {
				physicsBody->SetVelocity(sf::Vector2f(0, 0));
				physicsBody->SetAngularSpeed(0.f);
				physicsBody->GetCollisionGroups().reset();
				physicsBody->GetCollisionGroups().set(0, true);
			}

			if (auto rollingBall = ball->GetRollingBallBehaviour()) {
				rollingBall->ResetOmega();
			}
			_gameState.OnBallRemovedFromPocket(ballNumber);
		}
	}

	void EightBallPoolBehaviour::UpdateScoreboard() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetActivePlayerIndex(_gameState.GetActivePlayerIndex());
			if (_gameState.IsBallInHand() && !_isWaitingForBallsToStop) {
				scoreboard->ShowMessage("Foul! Ball in hand");
			}
			else {
				scoreboard->ShowMessage("");
			}
			scoreboard->SetPlayerBallType(0, _gameState.GetPlayerBallType(0));
			scoreboard->SetPlayerBallType(1, _gameState.GetPlayerBallType(1));
			if (_gameState.IsGameOver()) {
				scoreboard->ShowMessage(fmt::format("Player {} wins", _gameState.GetWinnerIndex() + 1));
			}
		}
	}

	void EightBallPoolBehaviour::UpdateScoreboardTimer() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetRemainingTurnTime(_remainingTurnTime);
		}
	}

	void EightBallPoolBehaviour::OnBallCollision(
	    std::shared_ptr<PhysicsBodyBehaviour> ballBody, int ballIndex, const IntersectionDetails& intersection) {
		auto node1 = intersection.wNode1.lock();
		auto node2 = intersection.wNode2.lock();
		if (!node1 || !node2) {
			return;
		}
		// looks like shitty workaround
		auto railRect = node1->GetVisual<RectangleShapeVisual>();
		if (!railRect) {
			railRect = node2->GetVisual<RectangleShapeVisual>();
		}
		if (railRect) {
			_gameState.OnBallCollideRail(ballIndex);
			return;
		}

		if (ballIndex == 0) {
			auto ballIndex1 = node1->FindBehaviour<BilliardBallBehaviour>()->GetBallNumber();
			auto ballIndex2 = node2->FindBehaviour<BilliardBallBehaviour>()->GetBallNumber();
			_gameState.OnCueBallCollideBall(std::max(ballIndex1, ballIndex2));
			return;
		}
	}

	sf::FloatRect EightBallPoolBehaviour::GetBallInHandRect() const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->GetGlobalBounds();
		}
		return sf::FloatRect();
	}

	sf::FloatRect EightBallPoolBehaviour::GetKitchenRect() const {
		if (auto tableRect = _tableRect.Get()) {
			auto rect = tableRect->GetGlobalBounds();
			auto radius = GetBallRadius();
			rect.size.x *= 0.25f;
			rect.size.x += radius;
			return rect;
		}
		return sf::FloatRect();
	}

	float EightBallPoolBehaviour::GetBallRadius() const {
		if (_ballsBehaviours.empty()) {
			return 0.f;
		}
		auto ball = _ballsBehaviours.begin()->second.Get();
		if (!ball) {
			return 0.f;
		}
		return ball->GetRadius();
	}

	sf::Vector2f EightBallPoolBehaviour::GetTableCenter() const {
		if (auto tableRect = _tableRect.Get()) {
			return tableRect->GetGlobalBounds().size * 0.5f;
		}
		return sf::Vector2f();
	}

	void EightBallPoolBehaviour::OnBallInHandGrab() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
	}

	void EightBallPoolBehaviour::OnBallInHandRelease() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
			cueBehaviour->ApplyCueTransform();
		}
	}
} // namespace Billiard
