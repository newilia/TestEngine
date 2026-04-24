#include "Engine/Visual/ConvexShapeVisual.h"

#include "Engine/App/Utils.h"

ConvexShapeVisual::ConvexShapeVisual(sf::ConvexShape* convex) : ShapeVisualBase(convex), _convex(convex) {}

bool ConvexShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _convex && utils::IsPointInsideOfShape(windowPosition, _convex);
}
