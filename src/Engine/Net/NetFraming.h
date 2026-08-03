#pragma once

#include <SFML/Network.hpp>

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace Engine::Net {

	[[nodiscard]] bool WriteLengthPrefixed(sf::TcpSocket& socket, std::span<const std::uint8_t> payload);

	[[nodiscard]] bool WriteLengthPrefixed(sf::TcpSocket& socket, const std::string& payload);

	// Appends available socket bytes to buffer and extracts complete length-prefixed frames.
	void DrainSocketIntoBuffer(sf::TcpSocket& socket, std::vector<std::uint8_t>& buffer);

	[[nodiscard]] std::optional<std::vector<std::uint8_t>> TryPopLengthPrefixedFrame(std::vector<std::uint8_t>& buffer);

} // namespace Engine::Net
