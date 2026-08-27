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
		void CreateSession(std::string serverHost, int serverPort);
		void JoinSession(std::string serverHost, int serverPort);
		void PollIncomingMessages();

		void SendTurnStarted(std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot);

		[[nodiscard]] bool HasPendingEvent() const;
		[[nodiscard]] BilliardNetworkEvent PopEvent();

		[[nodiscard]] std::uint32_t GetClientId() const;
		[[nodiscard]] int GetLocalPlayerIndex() const;
		[[nodiscard]] bool IsSessionReady() const;
		[[nodiscard]] bool IsWaitingForOpponent() const;
		[[nodiscard]] bool IsPassiveTurn(int activePlayerIndex) const;
		[[nodiscard]] bool IsLocalAuthority(int activePlayerIndex) const;

		void BeginMatch();
		[[nodiscard]] std::uint32_t GetNetworkTurnId() const;
		void OnRemoteTurnResultReceived();

		void OnLocalCueHit(int shootingPlayerIndex);
		void OnBallInHandDragStarted();
		bool TryAdvanceAimSendTick(float deltaSeconds);
		bool TryAdvanceTableSendTick(float deltaSeconds);

		void SendCueAimUpdate(int playerIndex, const TurnIntent& intent);
		void SendTableStateUpdate(int playerIndex, const TableSnapshot& snapshot);
		void SendBallInHandDragStarted(int playerIndex);
		void SendBallInHandDragEnded(int playerIndex);
		void SendCueReleased(int playerIndex, const TurnIntent& intent);
		void SendTurnResult(int nextActivePlayer, const TableSnapshot& snapshot, const RulesSnapshot& rules);

	private:
		enum class PendingRequest
		{
			None,
			CreateSession,
			JoinSession,
		};

		[[nodiscard]] bool EnsureConnected(const char* actionName, std::string serverHost, int serverPort);
		void EnqueueEvent(BilliardNetworkEvent event);

	private:
		std::string _playerName = "Player";
		std::unique_ptr<Engine::Net::TcpClient> _client;
		PendingRequest _pendingRequest = PendingRequest::None;
		std::uint32_t _clientId = 0;
		bool _isSessionReady = false;
		bool _isWaitingForOpponent = false;
		std::uint32_t _networkTurnId = 0;
		int _shootingPlayerIndex = -1;
		float _aimSendAccumulator = 0.f;
		float _tableSendAccumulator = 0.f;
		BilliardNetworkEventQueue _events;
	};

} // namespace Billiard
