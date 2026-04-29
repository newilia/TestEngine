#include "common.h"

sf::Vector2f Segment::getDirVector() const {
	return end - start;
}
