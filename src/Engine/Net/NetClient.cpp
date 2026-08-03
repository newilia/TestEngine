#include "Engine/Net/NetClient.h"

#include <google/protobuf/message.h>

namespace Engine::Net {

	bool NetClient::Connect(const std::string& host, unsigned short port) {
		Disconnect();

		const auto addressOpt = sf::IpAddress::resolve(host);
		if (!addressOpt) {
			return false;
		}

		_socket.setBlocking(true);
		const auto status = _socket.connect(*addressOpt, port, sf::seconds(3));
		_socket.setBlocking(false);
		if (status == sf::Socket::Status::Done) {
			return true;
		}
		_socket.disconnect();
		return false;
	}

	void NetClient::Disconnect() {
		_socket.disconnect();
		_recvBuffer.clear();
	}

	bool NetClient::IsConnected() const {
		return _socket.getRemoteAddress().has_value();
	}

	bool NetClient::SendMessage(const google::protobuf::Message& message) {
		if (!IsConnected()) {
			return false;
		}

		std::string serialized;
		if (!message.SerializeToString(&serialized)) {
			return false;
		}
		return WriteLengthPrefixed(_socket, serialized);
	}

	bool NetClient::PollMessage(google::protobuf::Message& out) {
		if (!IsConnected()) {
			return false;
		}

		DrainSocketIntoBuffer(_socket, _recvBuffer);
		const auto frame = TryPopLengthPrefixedFrame(_recvBuffer);
		if (!frame) {
			return false;
		}
		return out.ParseFromArray(frame->data(), static_cast<int>(frame->size()));
	}

} // namespace Engine::Net
