#include "Engine/Visual/CircleShapeVisual.h"

#include "CircleShapeVisual.generated.hpp"

#include <algorithm>
#include <cstddef>

namespace {
	constexpr int kMinCirclePoints = 3;
	constexpr int kMaxCirclePoints = 512;
} // namespace

CircleShapeVisual::CircleShapeVisual() {}

sf::Shape* CircleShapeVisual::GetBaseShape() {
	return &_circle;
}

sf::CircleShape* CircleShapeVisual::GetShape() {
	return &_circle;
}

float CircleShapeVisual::GetRadius() const {
	return _circle.getRadius();
}

void CircleShapeVisual::SetRadius(float radius) {
	_circle.setRadius(radius);
}

int CircleShapeVisual::GetPointCount() const {
	return static_cast<int>(_circle.getPointCount());
}

void CircleShapeVisual::SetPointCount(int count) {
	const int clamped = std::clamp(count, kMinCirclePoints, kMaxCirclePoints);
	_circle.setPointCount(static_cast<std::size_t>(clamped));
}
