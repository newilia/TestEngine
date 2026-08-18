#pragma once

#include "BilliardNetworkEvent.h"
#include "BilliardRulesSnapshot.h"
#include "BilliardTableSnapshot.h"
#include "BilliardTurnIntent.h"
#include "Engine/Net/TcpClient.h"

#include <cstdint>
#include <memory>
#include <string>

namespace Billiard {

	class OnlineSession
	{
	public:
		static constexpr std::uint32_t kMatchSessionId = 1;

	public:
		void CreateSession();
		void JoinSession();
		void PollIncomingMessages();

		void SendCueAimUpdate(std::uint32_t turnId, int playerIndex, const TurnIntent& intent);
		void SendTableStateUpdate(std::uint32_t turnId, int playerIndex, const TableSnapshot& snapshot);
		void SendTurnStarted(std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot);
		void SendTurnResult(
		    std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot, const RulesSnapshot& rules);

		[[nodiscard]] bool HasPendingEvent() const;
		[[nodiscard]] BilliardNetworkEvent PopEvent();

		[[nodiscard]] std::uint32_t GetClientId() const;
		[[nodiscard]] int GetLocalPlayerIndex() const;
		[[nodiscard]] bool IsSessionReady() const;
		[[nodiscard]] bool IsWaitingForOpponent() const;

	private:
		enum class PendingRequest
		{
			None,
			CreateSession,
			JoinSession,
		};

		[[nodiscard]] bool EnsureConnected(const char* actionName);
		void EnqueueEvent(BilliardNetworkEvent event);

	private:
		std::string _serverHost = "127.0.0.1";
		int _serverPort = 7777;
		std::string _playerName = "Player";
		std::unique_ptr<Engine::Net::TcpClient> _client;
		PendingRequest _pendingRequest = PendingRequest::None;
		std::uint32_t _clientId = 0;
		int _localPlayerIndex = -1;
		bool _isSessionReady = false;
		bool _isWaitingForOpponent = false;
		BilliardNetworkEventQueue _events;
	};

} // namespace Billiard
