#include "Engine/Visual/PolygonShapeVisual.h"

#include "Engine/App/Utils.h"

PolygonShapeVisual::PolygonShapeVisual(sf::Shape* shape) : ShapeVisualBase(shape) {}

bool PolygonShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return Utils::IsPointInsideOfShape(windowPosition, GetShape());
}
