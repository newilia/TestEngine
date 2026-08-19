#include "EightBallPoolController.h"

#include "EightBallPoolController.generated.hpp"
#include "Engine/Editor/Editor.h"

namespace Billiard {

	namespace {
		constexpr float kOutOfTablePocketMargin = 200.f;
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

	void EightBallPoolController::ConfigureHotSeatMatchLoop() {
		_player0Kind = PlayerKind::LocalHuman;
		_player1Kind = PlayerKind::LocalHuman;
		_player0IsLocalAuthority = true;
		_player1IsLocalAuthority = true;
		_player0Name = "Player 1";
		_player1Name = "Player 2";
		ConfigureMatchLoop();
	}

	void EightBallPoolController::ConfigureOnlineMatchLoop() {
		const int localPlayerIndex = _onlineSession->GetLocalPlayerIndex();
		_player0Kind = localPlayerIndex == 0 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman;
		_player1Kind = localPlayerIndex == 1 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman;
		_player0IsLocalAuthority = localPlayerIndex == 0;
		_player1IsLocalAuthority = localPlayerIndex == 1;
		_player0Name = localPlayerIndex == 0 ? "You" : "Opponent";
		_player1Name = localPlayerIndex == 1 ? "You" : "Opponent";
		ConfigureMatchLoop();
	}

	bool EightBallPoolController::IsLocalAuthority() const {
		return !_onlineSession || _onlineSession->IsLocalAuthority(_gameState.GetActivePlayerIndex());
	}

	bool EightBallPoolController::IsPassiveTurn() const {
		return _onlineSession && _onlineSession->IsPassiveTurn(_gameState.GetActivePlayerIndex());
	}

	void EightBallPoolController::UpdateAimDisplayInputEnabled() {
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			bool enabled = !_isWaitingForBallsToStop && !_gameState.IsGameOver() && !IsPassiveTurn();
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
		if (!_onlineSession || IsPassiveTurn() || _isWaitingForBallsToStop || _gameState.IsGameOver()) {
			return;
		}
		auto cueBehaviour = _cueBehaviour.Get();
		if (!cueBehaviour) {
			return;
		}
		const auto intent =
		    cueBehaviour->BuildTurnIntent(_gameState.GetActivePlayerIndex(), _onlineSession->GetNetworkTurnId());
		_onlineSession->SendCueAimUpdate(_gameState.GetActivePlayerIndex(), intent);
	}

	void EightBallPoolController::SendTableStateUpdateToPeer() {
		if (!_onlineSession || IsPassiveTurn()) {
			return;
		}
		_onlineSession->SendTableStateUpdate(_onlineSession->GetLocalPlayerIndex(), _tablePresenter.CaptureSnapshot());
	}

	void EightBallPoolController::SendTableStateUpdateIfNeeded() {
		if (!_isWaitingForBallsToStop && !_isDraggingBallInHand) {
			return;
		}
		SendTableStateUpdateToPeer();
	}

	void EightBallPoolController::OnUpdate(const sf::Time& deltaTime) {
		_gameTimestamp += deltaTime;
		const float deltaSeconds = deltaTime.asSeconds();

		if (_onlineSession) {
			PollNetworkEvents();

			if (_waitingForGameStart && _onlineSession->IsWaitingForOpponent()) {
				if (auto scoreboard = _scoreboardBehaviour.Get()) {
					scoreboard->ShowMessage("Waiting for opponent...");
				}
			}

			if (IsLocalAuthority() && !_isWaitingForBallsToStop &&
			    _onlineSession->TryAdvanceAimSendTick(deltaSeconds)) {
				SendCueAimUpdateIfNeeded();
			}

			if (IsLocalAuthority() && (_isWaitingForBallsToStop || _isDraggingBallInHand) &&
			    _onlineSession->TryAdvanceTableSendTick(deltaSeconds)) {
				SendTableStateUpdateIfNeeded();
			}
		}

		if (_isWaitingForBallsToStop) {
			if (!IsPassiveTurn()) {
				CheckBallsOutOfBounds();
			}
			if (!IsPassiveTurn() && !_tablePresenter.AreBallsMoving()) {
				OnBallsStopped();
			}
		}
		else {
			_matchLoop.OnUpdate(deltaTime, _gameState, _cueBehaviour.Get().get());
			if (_remainingTurnTime > 0.f) {
				_remainingTurnTime = std::max(0.f, _remainingTurnTime - deltaSeconds);
				if (_remainingTurnTime == 0.f) {
					if (!IsPassiveTurn()) {
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
		if (!_isWaitingForBallsToStop && !IsPassiveTurn()) {
			_matchLoop.OnEvent(event, _gameState);
		}
	}

	void EightBallPoolController::StartHotSeatGame() {
		_onlineSession = nullptr;
		_waitingForGameStart = false;
		ConfigureHotSeatMatchLoop();
		StartNewGame();
	}

	void EightBallPoolController::CreateServerSession() {
		_onlineSession = std::make_shared<OnlineSession>();
		_waitingForGameStart = true;
		_onlineSession->CreateSession();
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("Waiting for opponent...");
		}
	}

	void EightBallPoolController::JoinServerSession() {
		_onlineSession = std::make_shared<OnlineSession>();
		_waitingForGameStart = true;
		_onlineSession->JoinSession();
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("Joining session...");
		}
	}

	void EightBallPoolController::BeginOnlineMatch() {
		_onlineSession->BeginMatch();
		ConfigureOnlineMatchLoop();
		_waitingForGameStart = false;
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		StartNewGame();
	}

	void EightBallPoolController::PollNetworkEvents() {
		if (!_onlineSession) {
			return;
		}
		_onlineSession->PollIncomingMessages();

		while (_onlineSession->HasPendingEvent()) {
			const auto event = _onlineSession->PopEvent();
			switch (event.type) {
			case BilliardNetworkEventType::GameStarted:
				BeginOnlineMatch();
				break;
			case BilliardNetworkEventType::CueAimUpdate:
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
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
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
					_tablePresenter.ApplySnapshot(event.table);
					if (_tablePresenter.AreBallsMoving()) {
						if (auto cueBehaviour = _cueBehaviour.Get()) {
							cueBehaviour->PlayHideAnimation();
						}
						_isWaitingForBallsToStop = true;
					}
					else if (!_isWaitingForBallsToStop && _gameState.IsBallInHand()) {
						if (auto cueBehaviour = _cueBehaviour.Get()) {
							cueBehaviour->PlayHideAnimation();
						}
					}
					UpdateAimDisplayInputEnabled();
				}
				break;
			case BilliardNetworkEventType::TurnResult:
				_isWaitingForBallsToStop = false;
				_tablePresenter.ApplySnapshot(event.table);
				_gameState.ApplySnapshot(event.rules);
				ApplyTurnResolutionUI();
				_onlineSession->OnRemoteTurnResultReceived();
				if (!_gameState.IsGameOver()) {
					StartNewTurn();
				}
				break;
			}
		}
	}

	void EightBallPoolController::StartNewGame() {
		_gameTimestamp = sf::Time::Zero;
		_gameState.StartNewGame();
		StartNewTurn();

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
		if (_onlineSession) {
			ConfigureOnlineMatchLoop();
		}
		else {
			ConfigureMatchLoop();
		}
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter, _cueBehaviour.Get().get());
		UpdateAimDisplayInputEnabled();

		if (_onlineSession && _onlineSession->GetLocalPlayerIndex() == 0) {
			SendTableStateUpdateToPeer();
		}
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
		if (_onlineSession) {
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

	void EightBallPoolController::CheckBallsOutOfBounds() {
		if (_pocketsBehaviours.empty()) {
			return;
		}

		auto pocket = _pocketsBehaviours.front().Get();
		if (!pocket) {
			return;
		}

		for (const int ballNumber : _tablePresenter.CollectBallsOutsideExpandedTable(kOutOfTablePocketMargin)) {
			auto ball = _ballsBehaviours[ballNumber].Get();
			if (!ball) {
				continue;
			}
			pocket->PocketBall(*ball);
		}
	}

	void EightBallPoolController::OnBallFellInPocket(int ballNumber) {
		if (IsPassiveTurn()) {
			return;
		}
		_gameState.OnBallFellInPocket(ballNumber);
	}

	void EightBallPoolController::OnAimPointChanged(const sf::Vector2f& aimPoint) {
		if (IsPassiveTurn()) {
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
		if (IsPassiveTurn()) {
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
		if (IsPassiveTurn()) {
			return;
		}
		if (_onlineSession) {
			_onlineSession->OnLocalCueHit(_gameState.GetActivePlayerIndex());
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		_isWaitingForBallsToStop = true;
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

		if (_onlineSession) {
			_onlineSession->SendTurnResultIfLocalShooter(
			    _gameState.GetActivePlayerIndex(), _tablePresenter.CaptureSnapshot(), _gameState.ToSnapshot());
		}

		if (_gameState.IsGameOver()) {
			return;
		}

		StartNewTurn();
	}

	void EightBallPoolController::StartNewTurn() {
		_remainingTurnTime = _turnTimeLimit;
		_gameState.StartNewTurn();
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
		if (IsPassiveTurn()) {
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
		if (IsPassiveTurn()) {
			return;
		}
		_isDraggingBallInHand = true;
		if (_onlineSession) {
			_onlineSession->OnBallInHandDragStarted();
		}
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		SendTableStateUpdateIfNeeded();
	}

	void EightBallPoolController::OnBallInHandRelease() {
		if (IsPassiveTurn()) {
			return;
		}
		SendTableStateUpdateIfNeeded();
		_isDraggingBallInHand = false;
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
			cueBehaviour->ApplyCueTransform();
		}
	}
} // namespace Billiard
