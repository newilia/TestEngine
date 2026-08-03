#include "Engine/Net/NetFraming.h"

#include <cstring>

namespace Engine::Net {

	namespace {

		[[nodiscard]] std::uint32_t ReadLeU32(std::span<const std::uint8_t> bytes) {
			Verify(bytes.size() >= 4);
			std::uint32_t value = 0;
			std::memcpy(&value, bytes.data(), sizeof(value));
			return value;
		}

		[[nodiscard]] std::array<std::uint8_t, 4> WriteLeU32(std::uint32_t value) {
			std::array<std::uint8_t, 4> bytes{};
			std::memcpy(bytes.data(), &value, sizeof(value));
			return bytes;
		}

	} // namespace

	bool WriteLengthPrefixed(sf::TcpSocket& socket, std::span<const std::uint8_t> payload) {
		const auto header = WriteLeU32(static_cast<std::uint32_t>(payload.size()));
		std::array<sf::TcpSocket::Status, 2> statuses{};
		std::size_t sent = 0;

		statuses[0] = socket.send(header.data(), header.size(), sent);
		if (statuses[0] != sf::Socket::Status::Done || sent != header.size()) {
			return false;
		}

		sent = 0;
		statuses[1] = socket.send(payload.data(), payload.size(), sent);
		return statuses[1] == sf::Socket::Status::Done && sent == payload.size();
	}

	bool WriteLengthPrefixed(sf::TcpSocket& socket, const std::string& payload) {
		return WriteLengthPrefixed(socket,
		    std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(payload.data()), payload.size()});
	}

	void DrainSocketIntoBuffer(sf::TcpSocket& socket, std::vector<std::uint8_t>& buffer) {
		std::array<std::uint8_t, 4096> chunk{};
		while (true) {
			std::size_t received = 0;
			const auto status = socket.receive(chunk.data(), chunk.size(), received);
			if (status == sf::Socket::Status::Done) {
				buffer.insert(buffer.end(), chunk.begin(), chunk.begin() + static_cast<std::ptrdiff_t>(received));
				continue;
			}
			if (status == sf::Socket::Status::NotReady || status == sf::Socket::Status::Disconnected) {
				return;
			}
			return;
		}
	}

	std::optional<std::vector<std::uint8_t>> TryPopLengthPrefixedFrame(std::vector<std::uint8_t>& buffer) {
		if (buffer.size() < 4) {
			return std::nullopt;
		}

		const std::uint32_t payloadLength = ReadLeU32(std::span<const std::uint8_t>{buffer.data(), 4});
		const std::size_t frameSize = 4 + static_cast<std::size_t>(payloadLength);
		if (buffer.size() < frameSize) {
			return std::nullopt;
		}

		std::vector<std::uint8_t> payload(payloadLength);
		if (payloadLength > 0) {
			std::memcpy(payload.data(), buffer.data() + 4, payloadLength);
		}
		buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(frameSize));
		return payload;
	}

} // namespace Engine::Net
