#pragma once

#include <SFML/System/Vector2.hpp>

#include <functional>

namespace sf {
	class Event;
}

/// Distinguishes click from drag (6 px) before shape tools begin a stroke.
class EditorShapeStrokeGate
{
public:
	using OnStrokeStarted = std::function<void(sf::Vector2f world, sf::Vector2i pixel)>;
	using OnClick = std::function<void(sf::Vector2f world, sf::Vector2i pixel, bool isCtrlPressed)>;

	bool IsPending() const {
		return _pending;
	}

	bool ProcessEvent(const sf::Event& event, const OnStrokeStarted& onStrokeStarted, const OnClick& onClick);

private:
	static bool IsDragThresholdExceeded(sf::Vector2i from, sf::Vector2i to);

	bool _pending = false;
	sf::Vector2i _pressPixel{};
	sf::Vector2f _pressWorld{};
};
