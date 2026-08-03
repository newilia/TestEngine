#include "BilliardServerBehaviour.h"

#include "BilliardServerBehaviour.generated.hpp"
#include "BilliardSession.pb.h"

#include <fmt/format.h>

namespace Billiard {

	bool BilliardServerBehaviour::EnsureConnected(const char* actionName) {
		if (!_client) {
			_client = std::make_unique<Engine::Net::NetClient>();
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

	void BilliardServerBehaviour::CreateSession() {
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

	void BilliardServerBehaviour::JoinSession() {
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

	void BilliardServerBehaviour::OnUpdate(const sf::Time& /*deltaTime*/) {
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
