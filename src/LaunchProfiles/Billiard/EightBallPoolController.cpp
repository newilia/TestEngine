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
	}

	void EightBallPoolController::OnDeinit() {
		EventHandlerBehaviourBase::OnDeinit();
		_matchLoop.OnTurnEnded();
		UnsubscribeAll();
	}

	void EightBallPoolController::ConfigureMatchLoop(const MatchLoopConfig& config) {
		_matchLoop.Configure(config.slots, config.isLocalAuthorityForRemoteSlot);
	}

	std::shared_ptr<BilliardBallBehaviour> EightBallPoolController::GetCueBall() {
		if (Verify(!_ballsBehaviours.empty())) {
			return _ballsBehaviours[0].Get();
		}
		return nullptr;
	}

	void EightBallPoolController::BindPlayerAgentRuntimeDeps() {
		std::weak_ptr<BilliardCueBehaviour> cue;
		if (auto resolvedCue = _cueBehaviour.Get()) {
			cue = resolvedCue;
		}
		std::weak_ptr<BilliardBallBehaviour> cueBall = GetCueBall();
		_matchLoop.BindRuntimeDeps(cue, cueBall);
	}

	void EightBallPoolController::ConfigureHotSeatMatchLoop() {
		MatchLoopConfig config;
		config.slots[0] = {PlayerKind::LocalHuman, "Player 1"};
		config.slots[1] = {PlayerKind::LocalHuman, "Player 2"};
		ConfigureMatchLoop(config);
	}

	void EightBallPoolController::ConfigureOnlineMatchLoop() {
		assert(_onlineSession && _onlineSession->IsSessionReady());
		const int localPlayerIndex = _onlineSession->GetLocalPlayerIndex();
		MatchLoopConfig config;
		config.slots[0] = {localPlayerIndex == 0 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman,
		    localPlayerIndex == 0 ? "You" : "Opponent"};
		config.slots[1] = {localPlayerIndex == 1 ? PlayerKind::LocalHuman : PlayerKind::RemoteHuman,
		    localPlayerIndex == 1 ? "You" : "Opponent"};
		config.isLocalAuthorityForRemoteSlot = {localPlayerIndex == 0, localPlayerIndex == 1};
		ConfigureMatchLoop(config);
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
				Subscribe(pocket->GetOnBallPocketedSignal(), [this, pocketRef](int ballNumber) {
					OnBallFellInPocket(ballNumber, pocketRef.Get());
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
			Subscribe(cueBehaviour->GetOnAimChangedSignal(), [this]() {
				OnCueAimChanged();
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
			CheckBallsOutOfTableBounds();
			if (!_tablePresenter.AreBallsMoving() && !IsPassiveTurn()) {
				OnBallsStopped();
			}
		}
		else {
			_matchLoop.OnUpdate(deltaTime, _gameState);
		}

		UpdateCountdownTimer(deltaSeconds);
	}

	void EightBallPoolController::UpdateCountdownTimer(float deltaSeconds) {
		if (_gameState.IsGameOver()) {
			return;
		}
		if (_isWaitingForBallsToStop) {
			return;
		}

		if (_remainingTurnTime > 0.f) {
			_remainingTurnTime = std::max(0.f, _remainingTurnTime - deltaSeconds);
			if (_remainingTurnTime == 0.f) {
				OnTurnTimeOver();
			}
			UpdateScoreboardTimer();
		}
	}

	void EightBallPoolController::OnTurnTimeOver() {
		if (IsPassiveTurn()) {
			return;
		}

		_gameState.OnTurnTimeOver();
		_matchLoop.OnTurnEnded();

		if (_onlineSession) {
			_onlineSession->SendTurnResult(
			    _gameState.GetActivePlayerIndex(), _tablePresenter.CaptureSnapshot(), _gameState.ToSnapshot());
		}

		StartNewTurn();
		UpdateScoreboard();
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
		_onlineSession->CreateSession(_serverHost, _serverPort);
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("Waiting for opponent...");
		}
	}

	void EightBallPoolController::JoinServerSession() {
		_onlineSession = std::make_shared<OnlineSession>();
		_waitingForGameStart = true;
		_onlineSession->JoinSession(_serverHost, _serverPort);
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
					}
					if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
						aimDisplay->SetAimPoint({event.lateralSpin, event.verticalSpin});
						aimDisplay->Show();
					}
				}
				break;
			case BilliardNetworkEventType::BallInHandDragStarted:
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
					OnBallInHandGrab();
				}
				break;
			case BilliardNetworkEventType::BallInHandDragEnded:
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
					OnBallInHandRelease();
				}
				break;
			case BilliardNetworkEventType::CueReleased:
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
					if (auto cueBehaviour = _cueBehaviour.Get()) {
						TurnIntent intent;
						intent.directionAngle = event.directionAngle;
						intent.pullDistance = event.pullDistance;
						intent.lateralSpin = event.lateralSpin;
						intent.verticalSpin = event.verticalSpin;
						cueBehaviour->SetInputEnabled(true); // todo check
						cueBehaviour->ApplyShotIntent(intent);
					}
				}
				break;
			case BilliardNetworkEventType::TableStateUpdate:
				if (event.playerIndex != _onlineSession->GetLocalPlayerIndex()) {
					_tablePresenter.ApplySnapshot(event.table);
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
		InitScoreboard();
		InitPockets();
		SpawnBalls();
		InitCue();
		BindPlayerAgentRuntimeDeps();
		InitAimGuideLine();
		InitSubscriptions();
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter);

		_gameTimestamp = sf::Time::Zero;
		_gameState.StartNewGame();
		StartNewTurn();
		if (_onlineSession && _onlineSession->GetLocalPlayerIndex() == 0) { // is host
			SendTableStateUpdateToPeer();
		}
	}

	void EightBallPoolController::SpawnBalls() {
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
	}

	void EightBallPoolController::InitPockets() {
		for (auto& pocket : _pocketsBehaviours) {
			if (auto pocketBehaviour = pocket.Get()) {
				pocketBehaviour->Reset();
			}
		}
		_tablePresenter.SetPockets(_pocketsBehaviours);
	}

	void EightBallPoolController::InitScoreboard() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->Reset();
			scoreboard->SetPlayerName(0, _matchLoop.GetSlots()[0].displayName);
			scoreboard->SetPlayerName(1, _matchLoop.GetSlots()[1].displayName);
		}
	}

	void EightBallPoolController::InitCue() {
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			if (auto ball = GetCueBall()) {
				cueBehaviour->SetCueBall(ball);
			}
		}
	}

	void EightBallPoolController::InitAimGuideLine() {
		auto aimGuideLine = _aimGuideLineBehaviour.Get();
		auto cueBehaviour = _cueBehaviour.Get();
		if (!aimGuideLine || !cueBehaviour) {
			return;
		}
		aimGuideLine->SetCueBallIndex(0);
		aimGuideLine->SetDirectionAngle(cueBehaviour->GetActualBallDirectionAngle());
		aimGuideLine->Show();

		std::vector<std::weak_ptr<BilliardBallBehaviour>> balls;
		for (const auto& [ballNumber, ballRef] : _ballsBehaviours) {
			balls.push_back(ballRef.Get());
		}
		aimGuideLine->SetBalls(balls);
	}

	void EightBallPoolController::CheckBallsOutOfTableBounds() {
		for (const auto& [ballNumber, ballRef] : _ballsBehaviours) {
			if (_gameState.IsBallPocketed(ballNumber)) {
				continue;
			}
			if (_tablePresenter.IsBallOutsideExpandedTable(ballNumber, kOutOfTablePocketMargin)) {
				OnBallFellInPocket(ballNumber, nullptr);
			}
		}
	}

	void EightBallPoolController::OnBallFellInPocket(int ballNumber, shared_ptr<BilliardPocketBehaviour> pocket) {
		_gameState.OnBallFellInPocket(ballNumber);
		_tablePresenter.OnBallPocketed(ballNumber, pocket);
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->OnBallPocketed(ballNumber);
		}
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
		if (_onlineSession && !IsPassiveTurn()) {
			if (auto cueBehaviour = _cueBehaviour.Get()) {
				const auto intent = cueBehaviour->BuildTurnIntent(
				    _gameState.GetActivePlayerIndex(), _onlineSession->GetNetworkTurnId());
				_onlineSession->SendCueReleased(_gameState.GetActivePlayerIndex(), intent);
			}
		}

		if (auto ball = GetCueBall()) {
			ball->ResetBallInHand();
		}
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->ShowMessage("");
		}
		_gameState.OnShoot();
	}

	void EightBallPoolController::OnCueHit() {
		if (auto cue = _cueBehaviour.Get()) {
			cue->PlayHideAnimation();
		}
		if (auto aimGuideLine = _aimGuideLineBehaviour.Get()) {
			aimGuideLine->Hide();
		}

		_isWaitingForBallsToStop = true;

		if (IsPassiveTurn()) {
			return;
		}

		if (_onlineSession) {
			_onlineSession->OnLocalCueHit(_gameState.GetActivePlayerIndex());
		}
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

		if (_gameState.IsBallPocketed(0)) {
			_tablePresenter.RestoreBall(0);
			_gameState.OnBallRemovedFromPocket(0);
		}
		if (_gameState.IsBallPocketed(8)) {
			_tablePresenter.RestoreBall(8);
			_gameState.OnBallRemovedFromPocket(8);
		}

		if (_gameState.IsBallInHand()) {
			if (auto ball = GetCueBall()) {
				ball->SetBallInHand(_tablePresenter.GetBallInHandRect());
			}
		}

		StartNewTurn();
	}

	void EightBallPoolController::OnBallsStopped() {
		_isWaitingForBallsToStop = false;
		_gameState.OnBallsStopped();

		if (_onlineSession) {
			_onlineSession->SendTurnResult(
			    _gameState.GetActivePlayerIndex(), _tablePresenter.CaptureSnapshot(), _gameState.ToSnapshot());
		}

		ApplyTurnResolutionUI();
	}

	void EightBallPoolController::StartNewTurn() {
		_remainingTurnTime = _turnTimeLimit;
		_gameState.StartNewTurn();
		_matchLoop.OnTurnStarted(_gameState, _tablePresenter);

		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PrepareForNewTurn();
		}
		if (auto aimDisplay = _aimDisplayBehaviour.Get()) {
			aimDisplay->ResetAimPoint();
		}
		if (auto aimGuideLine = _aimGuideLineBehaviour.Get()) {
			aimGuideLine->Show();
		}

		UpdateScoreboard();
		UpdateAimDisplayInputEnabled();
	}

	void EightBallPoolController::UpdateScoreboard() {
		if (auto scoreboard = _scoreboardBehaviour.Get()) {
			scoreboard->SetActivePlayerIndex(_gameState.GetActivePlayerIndex());
			if (_gameState.GetFoulKind() != FoulKind::None) {
				scoreboard->ShowFoulMessage(_gameState.GetFoulKind());
			}
			else if (!_waitingForGameStart) {
				scoreboard->ShowMessage("");
			}
			scoreboard->SetPlayerBallType(0, _gameState.GetPlayerBallType(0));
			scoreboard->SetPlayerBallType(1, _gameState.GetPlayerBallType(1));
			if (_gameState.IsGameOver()) {
				const auto& slots = _matchLoop.GetSlots();
				const auto& winnerName = slots[static_cast<std::size_t>(_gameState.GetWinnerIndex())].displayName;
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
		_isDraggingBallInHand = true;
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayHideAnimation();
		}
		if (auto aimGuideLine = _aimGuideLineBehaviour.Get()) {
			aimGuideLine->Hide();
		}

		if (IsPassiveTurn()) {
			return;
		}

		if (_onlineSession) {
			// todo combine these two calls into one
			_onlineSession->OnBallInHandDragStarted();
			_onlineSession->SendBallInHandDragStarted(_gameState.GetActivePlayerIndex());
		}
		SendTableStateUpdateIfNeeded();
	}

	void EightBallPoolController::OnBallInHandRelease() {
		_isDraggingBallInHand = false;
		if (auto cueBehaviour = _cueBehaviour.Get()) {
			cueBehaviour->PlayShowAnimation();
			cueBehaviour->ApplyCueTransform();
		}
		if (auto aimGuideLine = _aimGuideLineBehaviour.Get()) {
			aimGuideLine->Show();
		}

		if (IsPassiveTurn()) {
			return;
		}
		SendTableStateUpdateIfNeeded();
		if (_onlineSession) {
			_onlineSession->SendBallInHandDragEnded(_gameState.GetActivePlayerIndex());
		}
	}

	void EightBallPoolController::OnCueAimChanged() {
		auto aimGuideLine = _aimGuideLineBehaviour.Get();
		auto cueBehaviour = _cueBehaviour.Get();
		if (aimGuideLine && cueBehaviour) {
			aimGuideLine->SetDirectionAngle(cueBehaviour->GetActualBallDirectionAngle());
			aimGuideLine->Recalculate();
		}
	}
} // namespace Billiard
