#include "Engine/Visual/CircleShapeVisual.h"

#include "CircleShapeVisual.generated.hpp"

#include <algorithm>
#include <cstddef>

namespace {
	constexpr int kMinCirclePoints = 3;
	constexpr int kDefaultCirclePoints = 64;
	constexpr int kMaxCirclePoints = 512;
} // namespace

CircleShapeVisual::CircleShapeVisual() {
	_circle.setPointCount(kDefaultCirclePoints);
}

const sf::Shape* CircleShapeVisual::GetBaseShape() const {
	return &_circle;
}

float CircleShapeVisual::GetRadius() const {
	return _circle.getRadius();
}

void CircleShapeVisual::SetRadius(float radius) {
	_circle.setRadius(radius);
	_circle.setOrigin({radius, radius});
}

sf::Vector2f CircleShapeVisual::GetOrigin() const {
	return _circle.getOrigin();
}

void CircleShapeVisual::SetOrigin(const sf::Vector2f& origin) {
	_circle.setOrigin(origin);
}

int CircleShapeVisual::GetPointCount() const {
	return static_cast<int>(_circle.getPointCount());
}

void CircleShapeVisual::SetPointCount(int count) {
	const int clamped = std::clamp(count, kMinCirclePoints, kMaxCirclePoints);
	_circle.setPointCount(static_cast<std::size_t>(clamped));
}
