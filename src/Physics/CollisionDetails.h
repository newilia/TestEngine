#pragma once
#include <SFML/System/Vector2.hpp>

struct CollisionDetails {
	sf::Vector2f point;
	sf::Vector2f normalizedTangent;
};

struct SegmentIntersectionPoints {
	sf::Vector2f p1;
	std::optional<sf::Vector2f> p2 = std::nullopt; // only for collinear segments
};