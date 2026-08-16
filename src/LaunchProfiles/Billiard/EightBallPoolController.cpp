#include "EightBallPoolController.h"

#include "EightBallPoolController.generated.hpp"
#include "Engine/Editor/Editor.h"

namespace Billiard {

	void EightBallPoolController::OnInit() {
		EventHandlerBehaviourBase::OnInit();
		Engine::Editor::GetInstance().SetCameraPanOnRightClickEnabled(false);
		_tablePresenter.SetTableRect(_tableRect);
		ConfigureMatchLoop();
		InitSubscriptions();
	}

	void EightBallPoolController::OnDeinit() {
		EventHandlerBehaviourBase::OnDeinit();
		_matchLoop.OnTurnEnded();
		UnsubscribeAll();
	}

	void EightBallPoolController::ConfigureMatchLoop() {
		std::array<PlayerSlotConfig, 2> slots;
		slots[0] = {_player0Kind, _player0Name};
		slots[1] = {_player1Kind, _player1Name};

		PlayerAgentDeps deps;
		if (auto cue = _cueBehaviour.Get()) {
			deps.cue = cue;
		}
		if (auto cueBall = _ballsBehaviours[0].Get()) {
			deps.cueBall = cueBall;
		}
		deps.isLocalAuthorityForRemoteSlot = {_player0IsLocalAuthority, _player1IsLocalAuthority};
		_matchLoop.Configure(slots, deps);
	}

	void EightBallPoolController::UpdateAimDisplayInputEnabled() {
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			bool enabled = !_isWaitingForBallsToStop && !_gameState.IsGameOver();
			if (enabled) {
				if (auto* agent = _matchLoop.GetActiveAgent(_gameState)) {
					enabled = agent->WantsInput();
				}
				else {
					enabled = false;
				}
			}
			aimDisplay->SetInputEnabled(enabled);
		}
	}

	void EightBallPoolController::InitSubscriptions() {
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

	void EightBallPoolController::OnUpdate(const sf::Time& deltaTime) {
		_gameTimestamp += deltaTime;

		if (_isWaitingForBallsToStop) {
			if (!_tablePresenter.AreBallsMoving()) {
				OnBallsStopped();
			}
		}
		else {
			_matchLoop.OnUpdate(deltaTime, _gameState, _cueBehaviour.Get().get());
			if (_remainingTurnTime > 0.f) {
				_remainingTurnTime = std::max(0.f, _remainingTurnTime - deltaTime.asSeconds());
				if (_remainingTurnTime == 0.f) {
					_gameState.OnTurnTimeOver();
					_matchLoop.OnTurnEnded();
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

	void EightBallPoolController::OnEvent(const sf::Event& event) {
		if (!_isWaitingForBallsToStop) {
			_matchLoop.OnEvent(event, _gameState);
		}
	}

	void EightBallPoolController::StartNewGame() {
		_gameTimestamp = sf::Time::Zero;
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
						ballBehaviour->SetBallInHand(_tablePresenter.GetKitchenRect());
					}
				}
			}
		}

		_tablePresenter.SetBalls(_ballsBehaviours);
		SpawnCue();
		InitSubscriptions();
		ConfigureMatchLoop();
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter, _cueBehaviour.Get().get());
		UpdateAimDisplayInputEnabled();
	}

	void EightBallPoolController::SpawnCue() {
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
						cueBehaviour->SetInputEnabled(false);
						SetCueOnBall(0);
					}
				}
			}
		}
		ConfigureMatchLoop();
	}

	void EightBallPoolController::SetCueOnBall(int ballNumber) {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			if (auto ball = _ballsBehaviours[ballNumber].Get()) {
				cueBehaviour->SetTargetBall(ball);
			}
		}
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			aimDisplay->ResetAimPoint();
		}
	}

	void EightBallPoolController::OnBallFellInPocket(int ballNumber) {
		_gameState.OnBallFellInPocket(ballNumber);
	}

	void EightBallPoolController::OnAimPointChanged(const sf::Vector2f& aimPoint) {
		if (auto* agent = _matchLoop.GetActiveAgent(_gameState)) {
			if (!agent->WantsInput()) {
				return;
			}
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->SetLateralPosition(aimPoint.x);
			cueBehaviour->SetVerticalSpin(aimPoint.y);
		}
	}

	void EightBallPoolController::OnCueRelease() {
		if (auto ball = _ballsBehaviours[0].Get()) {
			ball->ResetBallInHand();
		}
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		_gameState.OnShoot();
	}

	void EightBallPoolController::OnCueHit() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		_isWaitingForBallsToStop = true;
		_matchLoop.OnTurnEnded();
		UpdateAimDisplayInputEnabled();
	}

	void EightBallPoolController::OnBallsStopped() {
		_isWaitingForBallsToStop = false;
		_gameState.OnBallsStopped();

		UpdateScoreboard();
		if (_gameState.IsGameOver()) {
			_matchLoop.OnTurnEnded();
			UpdateAimDisplayInputEnabled();
			return;
		}

		if (_gameState.IsCueBallPocketed()) {
			_tablePresenter.RestoreBall(0);
			_gameState.OnBallRemovedFromPocket(0);
		}
		if (_gameState.IsEightBallPocketed()) {
			_tablePresenter.RestoreBall(8);
			_gameState.OnBallRemovedFromPocket(8);
		}

		if (_gameState.IsBallInHand()) {
			if (auto ball = _ballsBehaviours[0].Get()) {
				ball->SetBallInHand(_tablePresenter.GetBallInHandRect());
			}
		}

		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
		}
		SetCueOnBall(0);

		StartNewTurn();
	}

	void EightBallPoolController::StartNewTurn() {
		_remainingTurnTime = _turnTimeLimit;
		_gameState.StartNewTurn();
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter, _cueBehaviour.Get().get());
		UpdateScoreboard();
		UpdateAimDisplayInputEnabled();
	}

	void EightBallPoolController::UpdateScoreboard() {
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
				const auto& winnerName = _gameState.GetWinnerIndex() == 0 ? _player0Name : _player1Name;
				scoreboard->ShowMessage(fmt::format("{} wins", winnerName));
			}
		}
	}

	void EightBallPoolController::UpdateScoreboardTimer() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetRemainingTurnTime(_remainingTurnTime);
		}
	}

	void EightBallPoolController::OnBallCollision(
	    std::shared_ptr<PhysicsBodyBehaviour> ballBody, int ballIndex, const IntersectionDetails& intersection) {
		auto node1 = intersection.wNode1.lock();
		auto node2 = intersection.wNode2.lock();
		if (!node1 || !node2) {
			return;
		}
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
		}
	}

	void EightBallPoolController::OnBallInHandGrab() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
	}

	void EightBallPoolController::OnBallInHandRelease() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
			cueBehaviour->ApplyCueTransform();
		}
	}
} // namespace Billiard
