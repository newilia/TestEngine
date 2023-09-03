#pragma once
#include <unordered_set>
#include <SFML/System/Vector2.hpp>

struct CollisionDetails {
	weak_ptr<AbstractBody> wBody1;
	weak_ptr<AbstractBody> wBody2;
	std::unordered_set<size_t> body1penetratingPoints;
	std::unordered_set<size_t> body2penetratingPoints;
	Segment intersection;
};

struct SegmentIntersectionPoints {
	sf::Vector2f p1;
	std::optional<sf::Vector2f> p2 = std::nullopt; // Only for collinear segments. Needed for define collision tangent
};