#include "Engine/Visual/ConvexShapeVisual.h"

#include "Engine/Core/Utils.h"

ConvexShapeVisual::ConvexShapeVisual(sf::ConvexShape* convex) : ShapeVisualBase(convex), _convex(convex) {}

bool ConvexShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return _convex && Utils::IsWorldPointInsideOfShape(worldPoint, _convex);
}
