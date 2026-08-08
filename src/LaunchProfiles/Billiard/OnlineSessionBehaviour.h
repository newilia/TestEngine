#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Net/TcpClient.h"

#include <memory>
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
