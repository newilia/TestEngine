#include "OnlineSession.h"

#include "BilliardNetworkMessages.h"
#include "BilliardSession.pb.h"

#include <fmt/format.h>

namespace Billiard {

	namespace {
		constexpr float kAimSendIntervalSeconds = 0.05f;
		constexpr float kTableSendIntervalSeconds = 0.1f;
	} // namespace

	void OnlineSession::SendCueAimUpdate(int playerIndex, const TurnIntent& intent) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillCueAimUpdateMsg(_networkTurnId, playerIndex, intent, *envelope.mutable_cue_aim_update());
		(void)_client->SendMessage(envelope);
	}

	void OnlineSession::SendTableStateUpdate(int playerIndex, const TableSnapshot& snapshot) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillTableStateUpdateMsg(_networkTurnId, playerIndex, snapshot, *envelope.mutable_table_state_update());
		(void)_client->SendMessage(envelope);
	}

	void OnlineSession::SendTurnStarted(std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillTurnStartedMsg(turnId, activePlayer, snapshot, *envelope.mutable_turn_started());
		(void)_client->SendMessage(envelope);
	}

	void OnlineSession::SendTurnResult(
	    int nextActivePlayer, const TableSnapshot& snapshot, const RulesSnapshot& rules) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillTurnResultMsg(_networkTurnId, nextActivePlayer, snapshot, rules, *envelope.mutable_turn_result());
		(void)_client->SendMessage(envelope);
		++_networkTurnId;
	}

	void OnlineSession::SendTurnResultIfLocalShooter(
	    int nextActivePlayer, const TableSnapshot& snapshot, const RulesSnapshot& rules) {
		if (_shootingPlayerIndex != GetLocalPlayerIndex()) {
			_shootingPlayerIndex = -1;
			return;
		}
		SendTurnResult(nextActivePlayer, snapshot, rules);
		_shootingPlayerIndex = -1;
	}

	bool OnlineSession::IsPassiveTurn(int activePlayerIndex) const {
		return activePlayerIndex != GetLocalPlayerIndex();
	}

	bool OnlineSession::IsLocalAuthority(int activePlayerIndex) const {
		return activePlayerIndex == GetLocalPlayerIndex();
	}

	void OnlineSession::BeginMatch() {
		_networkTurnId = 1;
		_shootingPlayerIndex = -1;
		_aimSendAccumulator = 0.f;
		_tableSendAccumulator = 0.f;
	}

	std::uint32_t OnlineSession::GetNetworkTurnId() const {
		return _networkTurnId;
	}

	void OnlineSession::OnRemoteTurnResultReceived() {
		++_networkTurnId;
	}

	void OnlineSession::OnLocalCueHit(int shootingPlayerIndex) {
		_shootingPlayerIndex = shootingPlayerIndex;
		_tableSendAccumulator = 0.f;
	}

	void OnlineSession::OnBallInHandDragStarted() {
		_tableSendAccumulator = 0.f;
	}

	bool OnlineSession::TryAdvanceAimSendTick(float deltaSeconds) {
		_aimSendAccumulator += deltaSeconds;
		if (_aimSendAccumulator >= kAimSendIntervalSeconds) {
			_aimSendAccumulator = 0.f;
			return true;
		}
		return false;
	}

	bool OnlineSession::TryAdvanceTableSendTick(float deltaSeconds) {
		_tableSendAccumulator += deltaSeconds;
		if (_tableSendAccumulator >= kTableSendIntervalSeconds) {
			_tableSendAccumulator = 0.f;
			return true;
		}
		return false;
	}

	bool OnlineSession::HasPendingEvent() const {
		return !_events.empty();
	}

	BilliardNetworkEvent OnlineSession::PopEvent() {
		auto event = _events.front();
		_events.pop_front();
		return event;
	}

	std::uint32_t OnlineSession::GetClientId() const {
		return _clientId;
	}

	int OnlineSession::GetLocalPlayerIndex() const {
		if (_clientId == 0) {
			return -1;
		}
		return static_cast<int>(_clientId) - 1;
	}

	bool OnlineSession::IsSessionReady() const {
		return _isSessionReady;
	}

	bool OnlineSession::IsWaitingForOpponent() const {
		return _isWaitingForOpponent;
	}

	bool OnlineSession::EnsureConnected(const char* actionName) {
		if (!_client) {
			_client = std::make_unique<Engine::Net::TcpClient>();
		}

		if (_client->IsConnected()) {
			return true;
		}

		if (!_client->Connect(_serverHost, static_cast<unsigned short>(_serverPort))) {
			fmt::print("[Net] {}: connect failed ({}:{})\n", actionName, _serverHost, _serverPort);
			return false;
		}

		fmt::print("[Net] {}: connected to {}:{}\n", actionName, _serverHost, _serverPort);
		return true;
	}

	void OnlineSession::EnqueueEvent(BilliardNetworkEvent event) {
		_events.push_back(std::move(event));
	}

	void OnlineSession::PollIncomingMessages() {
		if (!_client || !_client->IsConnected()) {
			return;
		}

		while (true) {
			billiard::Envelope envelope;
			if (!_client->PollMessage(envelope)) {
				break;
			}

			if (envelope.has_create_session_response()) {
				const auto& response = envelope.create_session_response();
				if (response.success()) {
					_clientId = response.client_id();
					_isWaitingForOpponent = true;
					fmt::print("[Net] CreateSession: success session_id={} client_id={}\n", response.session_id(),
					    response.client_id());
				}
				else {
					fmt::print("[Net] CreateSession: failed ({})\n", response.error_message());
				}
				_pendingRequest = PendingRequest::None;
				continue;
			}

			if (envelope.has_join_session_response()) {
				const auto& response = envelope.join_session_response();
				if (response.success()) {
					_clientId = response.client_id();
					fmt::print("[Net] JoinSession: success session_id={} client_id={}\n", response.session_id(),
					    response.client_id());
				}
				else {
					fmt::print("[Net] JoinSession: failed ({})\n", response.error_message());
				}
				_pendingRequest = PendingRequest::None;
				continue;
			}

			if (envelope.has_game_started()) {
				const auto& started = envelope.game_started();
				Verify(started.your_player_index() == GetLocalPlayerIndex());
				_isSessionReady = true;
				_isWaitingForOpponent = false;
				BilliardNetworkEvent event;
				event.type = BilliardNetworkEventType::GameStarted;
				event.playerIndex = started.your_player_index();
				EnqueueEvent(std::move(event));
				fmt::print("[Net] GameStarted: player_index={}\n", started.your_player_index());
				continue;
			}

			if (envelope.has_cue_aim_update()) {
				const auto& update = envelope.cue_aim_update();
				const auto intent = ParseCueAimUpdateMsg(update);
				BilliardNetworkEvent event;
				event.type = BilliardNetworkEventType::CueAimUpdate;
				event.turnId = update.turn_id();
				event.playerIndex = update.player_index();
				event.directionAngle = intent.directionAngle;
				event.pullDistance = intent.pullDistance;
				event.lateralSpin = intent.lateralSpin;
				event.verticalSpin = intent.verticalSpin;
				EnqueueEvent(std::move(event));
				continue;
			}

			if (envelope.has_table_state_update()) {
				const auto& update = envelope.table_state_update();
				BilliardNetworkEvent event;
				event.type = BilliardNetworkEventType::TableStateUpdate;
				event.turnId = update.turn_id();
				event.playerIndex = update.player_index();
				event.table = ParseTableSnapshotMsg(update.table());
				EnqueueEvent(std::move(event));
				continue;
			}

			if (envelope.has_turn_result()) {
				const auto& result = envelope.turn_result();
				BilliardNetworkEvent event;
				event.type = BilliardNetworkEventType::TurnResult;
				event.turnId = result.turn_id();
				event.nextActivePlayer = result.next_active_player();
				event.table = ParseTableSnapshotMsg(result.table());
				if (result.has_rules()) {
					event.rules = ParseRulesSnapshotMsg(result.rules());
				}
				EnqueueEvent(std::move(event));
			}
		}
	}

	void OnlineSession::CreateSession() {
		if (!EnsureConnected("CreateSession")) {
			return;
		}

		billiard::Envelope envelope;
		envelope.mutable_create_session_request()->set_player_name(_playerName);
		if (!_client->SendMessage(envelope)) {
			fmt::print("[Net] CreateSession: send failed\n");
			_pendingRequest = PendingRequest::None;
			return;
		}

		_pendingRequest = PendingRequest::CreateSession;
		fmt::print("[Net] CreateSession: request sent (player={})\n", _playerName);
	}

	void OnlineSession::JoinSession() {
		if (!EnsureConnected("JoinSession")) {
			return;
		}

		billiard::Envelope envelope;
		auto* request = envelope.mutable_join_session_request();
		request->set_session_id(kMatchSessionId);
		request->set_player_name(_playerName);
		if (!_client->SendMessage(envelope)) {
			fmt::print("[Net] JoinSession: send failed\n");
			_pendingRequest = PendingRequest::None;
			return;
		}

		_pendingRequest = PendingRequest::JoinSession;
		fmt::print("[Net] JoinSession: request sent (session_id={}, player={})\n", kMatchSessionId, _playerName);
	}

} // namespace Billiard
