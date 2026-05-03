#include "Engine/Visual/ConvexShapeVisual.h"

#include "ConvexShapeVisual.generated.hpp"
#include "Engine/Core/Utils.h"

ConvexShapeVisual::ConvexShapeVisual() {}

sf::Shape* ConvexShapeVisual::GetBaseShape() {
	return &_convex;
}

bool ConvexShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return Utils::IsWorldPointInsideOfShape(worldPoint, &_convex);
}

sf::ConvexShape* ConvexShapeVisual::GetShape() {
	return &_convex;
}

std::vector<sf::Vector2f> ConvexShapeVisual::GetPoints() const {
	std::vector<sf::Vector2f> out;
	const std::size_t n = _convex.getPointCount();
	out.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		out.push_back(_convex.getPoint(i));
	}
	return out;
}

void ConvexShapeVisual::SetPoints(const std::vector<sf::Vector2f>& points) {
	_convex.setPointCount(points.size());
	for (std::size_t i = 0; i < points.size(); ++i) {
		_convex.setPoint(i, points[i]);
	}
}
