#pragma once

#include "Engine/Net/NetFraming.h"

#include <SFML/Network.hpp>

#include <string>
#include <vector>

namespace google::protobuf {
	class Message;
}

namespace Engine::Net {

	class TcpClient
	{
	public:
		[[nodiscard]] bool Connect(const std::string& host, unsigned short port);
		void Disconnect();
		[[nodiscard]] bool IsConnected() const;

		[[nodiscard]] bool SendMessage(const google::protobuf::Message& message);
		[[nodiscard]] bool PollMessage(google::protobuf::Message& out);

	private:
		sf::TcpSocket _socket;
		std::vector<std::uint8_t> _recvBuffer;
	};

} // namespace Engine::Net
