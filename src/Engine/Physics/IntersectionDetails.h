#pragma once

#include "Engine/SceneNode.h"

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <optional>

struct Segment;

struct IntersectionDetails
{
	std::weak_ptr<SceneNode> wNode1;
	std::weak_ptr<SceneNode> wNode2;
	Segment intersection;
};

struct SegmentIntersectionPoints
{
	sf::Vector2f p1;
	std::optional<sf::Vector2f> p2 = std::nullopt;
};
