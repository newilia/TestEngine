#include "Engine/Visual/CircleShapeVisual.h"

#include "Engine/App/Utils.h"

CircleShapeVisual::CircleShapeVisual(sf::CircleShape* circle) : ShapeVisualBase(circle), _circle(circle) {}

bool CircleShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _circle && utils::IsPointInsideOfShape(windowPosition, _circle);
}
