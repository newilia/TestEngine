#pragma once

#include <memory>
#include <set>
#include <vector>
#include <list>
#include <algorithm>
#include <optional>
#include <SFML/Graphics.hpp>

using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::make_shared;
using std::dynamic_pointer_cast;

struct Segment {
	Segment() = default;
	Segment(sf::Vector2f start, sf::Vector2f end) : start(start), end(end) {};
	sf::Vector2f start, end;
};