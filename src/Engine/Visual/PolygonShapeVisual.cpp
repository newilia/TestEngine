#include "Engine/Visual/PolygonShapeVisual.h"

#include "Engine/Core/Utils.h"

PolygonShapeVisual::PolygonShapeVisual(sf::Shape* shape) : ShapeVisualBase(shape) {}

bool PolygonShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return Utils::IsWorldPointInsideOfShape(worldPoint, GetShape());
}
