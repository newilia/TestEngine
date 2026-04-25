#pragma once

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <vector>

using std::dynamic_pointer_cast;
using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;

struct Segment
{
	Segment() = default;
	Segment(sf::Vector2f start, sf::Vector2f end) : start(start), end(end) {};
	sf::Vector2f start, end;

	sf::Vector2f getDirVector() const { return end - start; }
};
