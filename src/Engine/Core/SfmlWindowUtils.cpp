#include "Engine/Core/SfmlWindowUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <SFML/Graphics/CircleShape.hpp>

namespace Utils {
	sf::CircleShape CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color) {
		sf::CircleShape circle;
		circle.setPosition(pos);
		circle.setRadius(radius);
		circle.setOrigin({radius, radius});
		circle.setFillColor(color);
		return circle;
	}

	void MaximizeWindow(const sf::RenderWindow& window) {
#ifdef _WIN32
		ShowWindow(static_cast<HWND>(window.getNativeHandle()), SW_MAXIMIZE);
#endif
	}

#ifdef _WIN32
	std::wstring Utf8ToWide(std::string_view utf8) {
		if (utf8.empty()) {
			return {};
		}
		const int byteCount = static_cast<int>(utf8.size());
		const int wideCount = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), byteCount, nullptr, 0);
		if (wideCount <= 0) {
			return {};
		}
		std::wstring wide(static_cast<size_t>(wideCount), L'\0');
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), byteCount, wide.data(), wideCount) <= 0) {
			return {};
		}
		return wide;
	}
#endif

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixel) {
		return window.mapPixelToCoords(pixel);
	}

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2f& pixel) {
		return MapWindowPixelToWorld(window, sf::Vector2i(pixel));
	}

	sf::Vector2i MapWorldToWindowPixel(const sf::RenderWindow& window, const sf::Vector2f& world) {
		return window.mapCoordsToPixel(world);
	}
} // namespace Utils
