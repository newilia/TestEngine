#pragma once

#include "SFML/Graphics.hpp"

#include <string_view>

namespace Utils {
	sf::CircleShape CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color);

	/// Системно разворачивает окно (Windows: `ShowWindow` + `SW_MAXIMIZE`); на других ОС — пусто.
	void MaximizeWindow(const sf::RenderWindow& window);

#ifdef _WIN32
	[[nodiscard]] std::wstring Utf8ToWide(std::string_view utf8);
#endif

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixel);
	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2f& pixel);
	sf::Vector2i MapWorldToWindowPixel(const sf::RenderWindow& window, const sf::Vector2f& world);
} // namespace Utils
