#include "common.h"

Segment::Segment(sf::Vector2f start, sf::Vector2f end) : start(start), end(end) {}

sf::Vector2f Segment::getDirVector() const {
	return end - start;
}
