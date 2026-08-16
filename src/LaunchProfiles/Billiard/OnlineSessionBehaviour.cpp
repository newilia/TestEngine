#include "OnlineSessionBehaviour.h"

#include "BilliardNetworkMessages.h"
#include "BilliardSession.pb.h"
#include "OnlineSessionBehaviour.generated.hpp"

#include <fmt/format.h>

namespace Billiard {

	void OnlineSessionBehaviour::SendTurnStarted(
	    std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillTurnStartedMsg(turnId, activePlayer, snapshot, *envelope.mutable_turn_started());
		(void)_client->SendMessage(envelope);
	}

	void OnlineSessionBehaviour::SendTurnResult(
	    std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot) {
		if (!_client || !_client->IsConnected()) {
			return;
		}
		billiard::Envelope envelope;
		FillTurnResultMsg(turnId, nextActivePlayer, snapshot, *envelope.mutable_turn_result());
		(void)_client->SendMessage(envelope);
	}

	std::optional<TableSnapshot> OnlineSessionBehaviour::PollRemoteTurnResult(
	    std::uint32_t& turnId, int& nextActivePlayer) {
		if (!_client || !_client->IsConnected()) {
			return std::nullopt;
		}
		billiard::Envelope envelope;
		if (!_client->PollMessage(envelope) || !envelope.has_turn_result()) {
			return std::nullopt;
		}
		const auto& result = envelope.turn_result();
		turnId = result.turn_id();
		nextActivePlayer = result.next_active_player();
		return ParseTableSnapshotMsg(result.table());
	}

	bool OnlineSessionBehaviour::EnsureConnected(const char* actionName) {
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

	void OnlineSessionBehaviour::CreateSession() {
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

	void OnlineSessionBehaviour::JoinSession() {
		if (!EnsureConnected("JoinSession")) {
			return;
		}

		billiard::Envelope envelope;
		auto* request = envelope.mutable_join_session_request();
		request->set_session_id(static_cast<std::uint32_t>(_sessionId));
		request->set_player_name(_playerName);
		if (!_client->SendMessage(envelope)) {
			fmt::print("[Net] JoinSession: send failed\n");
			_pendingRequest = PendingRequest::None;
			return;
		}

		_pendingRequest = PendingRequest::JoinSession;
		fmt::print("[Net] JoinSession: request sent (session_id={}, player={})\n", _sessionId, _playerName);
	}

	void OnlineSessionBehaviour::OnUpdate(const sf::Time& /*deltaTime*/) {
		if (!_client || _pendingRequest == PendingRequest::None) {
			return;
		}

		while (true) {
			billiard::Envelope envelope;
			if (!_client->PollMessage(envelope)) {
				break;
			}

			if (_pendingRequest == PendingRequest::CreateSession) {
				if (!envelope.has_create_session_response()) {
					continue;
				}

				const auto& response = envelope.create_session_response();
				if (response.success()) {
					fmt::print("[Net] CreateSession: success session_id={} client_id={}\n", response.session_id(),
					    response.client_id());
				}
				else {
					fmt::print("[Net] CreateSession: failed ({})\n", response.error_message());
				}
				_pendingRequest = PendingRequest::None;
				return;
			}

			if (_pendingRequest == PendingRequest::JoinSession) {
				if (!envelope.has_join_session_response()) {
					continue;
				}

				const auto& response = envelope.join_session_response();
				if (response.success()) {
					fmt::print("[Net] JoinSession: success session_id={} client_id={}\n", response.session_id(),
					    response.client_id());
				}
				else {
					fmt::print("[Net] JoinSession: failed ({})\n", response.error_message());
				}
				_pendingRequest = PendingRequest::None;
				return;
			}
		}
	}

} // namespace Billiard
