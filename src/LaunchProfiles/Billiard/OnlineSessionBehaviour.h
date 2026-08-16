#pragma once

#include "BilliardTableSnapshot.h"
#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Net/TcpClient.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace Billiard {

	class OnlineSessionBehaviour : public Behaviour
	{
		META_CLASS()

	public:
		void OnUpdate(const sf::Time& deltaTime) override;

		/// @method
		void CreateSession();

		/// @method
		void JoinSession();

		void SendTurnStarted(std::uint32_t turnId, int activePlayer, const TableSnapshot& snapshot);
		void SendTurnResult(std::uint32_t turnId, int nextActivePlayer, const TableSnapshot& snapshot);
		[[nodiscard]] std::optional<TableSnapshot> PollRemoteTurnResult(std::uint32_t& turnId, int& nextActivePlayer);

	private:
		enum class PendingRequest
		{
			None,
			CreateSession,
			JoinSession,
		};

		[[nodiscard]] bool EnsureConnected(const char* actionName);

		/// @property
		std::string _serverHost = "127.0.0.1";

		/// @property
		int _serverPort = 7777;

		/// @property
		int _sessionId = 1;

		/// @property
		std::string _playerName = "Player";

		std::unique_ptr<Engine::Net::TcpClient> _client;
		PendingRequest _pendingRequest = PendingRequest::None;
	};

} // namespace Billiard
