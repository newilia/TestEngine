#include "EightBallPoolController.h"

#include "EightBallPoolController.generated.hpp"
#include "Engine/Editor/Editor.h"

namespace Billiard {

	namespace {
		constexpr float kAimSendIntervalSeconds = 0.05f;
		constexpr float kTableSendIntervalSeconds = 0.1f;
	} // namespace

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

	void EightBallPoolController::ConfigureOnlineMatchLoop() {
		_player0Kind = _localPlayerIndex == 0 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman;
		_player1Kind = _localPlayerIndex == 1 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman;
		_player0IsLocalAuthority = _localPlayerIndex == 0;
		_player1IsLocalAuthority = _localPlayerIndex == 1;
		_player0Name = _localPlayerIndex == 0 ? "You" : "Opponent";
		_player1Name = _localPlayerIndex == 1 ? "You" : "Opponent";
		ConfigureMatchLoop();
	}

	bool EightBallPoolController::IsLocalAuthority() const {
		return !_isOnlineMatch || _gameState.GetActivePlayerIndex() == _localPlayerIndex;
	}

	void EightBallPoolController::UpdatePassiveTurnState() {
		_isPassiveTurn = _isOnlineMatch && !IsLocalAuthority();
	}

	void EightBallPoolController::UpdateAimDisplayInputEnabled() {
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			bool enabled = !_isWaitingForBallsToStop && !_gameState.IsGameOver() && !_isPassiveTurn;
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

	void EightBallPoolController::SendCueAimUpdateIfNeeded() {
		if (!_isOnlineMatch || _isPassiveTurn || _isWaitingForBallsToStop || _gameState.IsGameOver()) {
			return;
		}
		auto online = _onlineSession.Get();
		auto cueBehaviour = _cueBehaviour.Get();
		if (!online || !cueBehaviour) {
			return;
		}
		const auto intent = cueBehaviour->BuildTurnIntent(_gameState.GetActivePlayerIndex(), _networkTurnId);
		online->SendCueAimUpdate(_networkTurnId, _gameState.GetActivePlayerIndex(), intent);
	}

	void EightBallPoolController::SendTableStateUpdateIfNeeded() {
		if (!_isOnlineMatch || _isPassiveTurn || !_isWaitingForBallsToStop) {
			return;
		}
		auto online = _onlineSession.Get();
		if (!online) {
			return;
		}
		online->SendTableStateUpdate(
		    _networkTurnId, _gameState.GetActivePlayerIndex(), _tablePresenter.CaptureSnapshot());
	}

	void EightBallPoolController::OnUpdate(const sf::Time& deltaTime) {
		_gameTimestamp += deltaTime;

		if (_isOnlineMatch) {
			PollNetworkEvents();
			UpdatePassiveTurnState();

			if (auto online = _onlineSession.Get()) {
				if (_waitingForGameStart && online->IsWaitingForOpponent()) {
					if (auto scoreboard = _scoreboardBehaviour.Get()) {
						scoreboard->ShowMessage("Waiting for opponent...");
					}
				}
			}
		}

		const float deltaSeconds = deltaTime.asSeconds();
		if (_isOnlineMatch && IsLocalAuthority() && !_isWaitingForBallsToStop) {
			_aimSendAccumulator += deltaSeconds;
			if (_aimSendAccumulator >= kAimSendIntervalSeconds) {
				_aimSendAccumulator = 0.f;
				SendCueAimUpdateIfNeeded();
			}
		}

		if (_isOnlineMatch && IsLocalAuthority() && _isWaitingForBallsToStop) {
			_tableSendAccumulator += deltaSeconds;
			if (_tableSendAccumulator >= kTableSendIntervalSeconds) {
				_tableSendAccumulator = 0.f;
				SendTableStateUpdateIfNeeded();
			}
		}

		if (_isWaitingForBallsToStop) {
			if (!_isPassiveTurn && !_tablePresenter.AreBallsMoving()) {
				OnBallsStopped();
			}
		}
		else {
			_matchLoop.OnUpdate(deltaTime, _gameState, _cueBehaviour.Get().get());
			if (_remainingTurnTime > 0.f) {
				_remainingTurnTime = std::max(0.f, _remainingTurnTime - deltaSeconds);
				if (_remainingTurnTime == 0.f) {
					if (!_isPassiveTurn) {
						_gameState.OnTurnTimeOver();
					}
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
		if (!_isWaitingForBallsToStop && !_isPassiveTurn) {
			_matchLoop.OnEvent(event, _gameState);
		}
	}

	void EightBallPoolController::CreateServerSession() {
		auto online = _onlineSession.Get();
		if (!online) {
			return;
		}
		_isOnlineMatch = true;
		_waitingForGameStart = true;
		online->CreateSession();
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("Waiting for opponent...");
		}
	}

	void EightBallPoolController::JoinServerSession() {
		auto online = _onlineSession.Get();
		if (!online) {
			return;
		}
		_isOnlineMatch = true;
		_waitingForGameStart = true;
		online->JoinSession();
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("Joining session...");
		}
	}

	void EightBallPoolController::BeginOnlineMatch() {
		if (auto online = _onlineSession.Get()) {
			_localPlayerIndex = online->GetLocalPlayerIndex();
		}
		ConfigureOnlineMatchLoop();
		_waitingForGameStart = false;
		_networkTurnId = 1;
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		StartNewGame();
	}

	void EightBallPoolController::PollNetworkEvents() {
		auto online = _onlineSession.Get();
		if (!online) {
			return;
		}

		while (online->HasPendingEvent()) {
			const auto event = online->PopEvent();
			switch (event.type) {
			case BilliardNetworkEventType::GameStarted:
				BeginOnlineMatch();
				break;
			case BilliardNetworkEventType::CueAimUpdate:
				if (event.playerIndex != _localPlayerIndex) {
					if (auto cueBehaviour = _cueBehaviour.Get()) {
						cueBehaviour->SetDirectionAngle(event.directionAngle);
						cueBehaviour->SetDistanceFromTarget(event.pullDistance);
						cueBehaviour->SetLateralPosition(event.lateralSpin);
						cueBehaviour->SetVerticalSpin(event.verticalSpin);
						cueBehaviour->ApplyCueTransform();
						cueBehaviour->EnsureCueVisible();
					}
				}
				break;
			case BilliardNetworkEventType::TableStateUpdate:
				if (event.playerIndex != _localPlayerIndex) {
					_tablePresenter.ApplySnapshot(event.table);
					if (!_isWaitingForBallsToStop) {
						if (auto cueBehaviour = _cueBehaviour.Get()) {
							cueBehaviour->PlayHideAnimation();
						}
					}
					_isWaitingForBallsToStop = true;
					UpdateAimDisplayInputEnabled();
				}
				break;
			case BilliardNetworkEventType::TurnResult:
				if (_isOnlineMatch) {
					_isWaitingForBallsToStop = false;
					_tablePresenter.ApplySnapshot(event.table);
					_gameState.ApplySnapshot(event.rules);
					UpdatePassiveTurnState();
					ApplyTurnResolutionUI();
					++_networkTurnId;
					if (!_gameState.IsGameOver()) {
						StartNewTurn();
					}
				}
				break;
			}
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
		if (_isOnlineMatch) {
			ConfigureOnlineMatchLoop();
		}
		else {
			ConfigureMatchLoop();
		}
		UpdatePassiveTurnState();
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
		if (_isOnlineMatch) {
			ConfigureOnlineMatchLoop();
		}
		else {
			ConfigureMatchLoop();
		}
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
		if (_isPassiveTurn) {
			return;
		}
		_gameState.OnBallFellInPocket(ballNumber);
	}

	void EightBallPoolController::OnAimPointChanged(const sf::Vector2f& aimPoint) {
		if (_isPassiveTurn) {
			return;
		}
		if (auto* agent = _matchLoop.GetActiveAgent(_gameState)) {
			if (!agent->WantsInput()) {
				return;
			}
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->SetLateralPosition(aimPoint.x);
			cueBehaviour->SetVerticalSpin(aimPoint.y);
		}
		SendCueAimUpdateIfNeeded();
	}

	void EightBallPoolController::OnCueRelease() {
		if (_isPassiveTurn) {
			return;
		}
		if (auto ball = _ballsBehaviours[0].Get()) {
			ball->ResetBallInHand();
		}
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		_gameState.OnShoot();
	}

	void EightBallPoolController::OnCueHit() {
		if (_isPassiveTurn) {
			return;
		}
		_shootingPlayerIndex = _gameState.GetActivePlayerIndex();
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		_isWaitingForBallsToStop = true;
		_tableSendAccumulator = 0.f;
		_matchLoop.OnTurnEnded();
		UpdateAimDisplayInputEnabled();
		SendTableStateUpdateIfNeeded();
	}

	void EightBallPoolController::ApplyTurnResolutionUI() {
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
			cueBehaviour->EnsureCueVisible();
		}
		SetCueOnBall(0);
	}

	void EightBallPoolController::OnBallsStopped() {
		_isWaitingForBallsToStop = false;
		_gameState.OnBallsStopped();

		ApplyTurnResolutionUI();

		if (_isOnlineMatch && _shootingPlayerIndex == _localPlayerIndex) {
			if (auto online = _onlineSession.Get()) {
				const auto rules = _gameState.ToSnapshot();
				online->SendTurnResult(
				    _networkTurnId, _gameState.GetActivePlayerIndex(), _tablePresenter.CaptureSnapshot(), rules);
			}
			++_networkTurnId;
		}
		_shootingPlayerIndex = -1;

		if (_gameState.IsGameOver()) {
			return;
		}

		StartNewTurn();
	}

	void EightBallPoolController::StartNewTurn() {
		_remainingTurnTime = _turnTimeLimit;
		_gameState.StartNewTurn();
		UpdatePassiveTurnState();
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter, _cueBehaviour.Get().get());
		if (IsLocalAuthority() && !_isWaitingForBallsToStop) {
			SetCueOnBall(0);
			if (auto cueBehaviour = _cueBehaviour.Get()) {
				cueBehaviour->EnsureCueVisible();
			}
		}
		UpdateScoreboard();
		UpdateAimDisplayInputEnabled();
	}

	void EightBallPoolController::UpdateScoreboard() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetActivePlayerIndex(_gameState.GetActivePlayerIndex());
			if (_gameState.IsBallInHand() && !_isWaitingForBallsToStop) {
				scoreboard->ShowMessage("Foul! Ball in hand");
			}
			else if (!_waitingForGameStart) {
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
		if (_isPassiveTurn) {
			return;
		}
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
		if (_isPassiveTurn) {
			return;
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
	}

	void EightBallPoolController::OnBallInHandRelease() {
		if (_isPassiveTurn) {
			return;
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
			cueBehaviour->ApplyCueTransform();
		}
	}
} // namespace Billiard
