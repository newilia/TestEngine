#include "Engine/Visual/CircleShapeVisual.h"

#include "Engine/Core/Utils.h"

CircleShapeVisual::CircleShapeVisual(sf::CircleShape* circle) : ShapeVisualBase(circle), _circle(circle) {}

bool CircleShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return _circle && Utils::IsWorldPointInsideOfShape(worldPoint, _circle);
}
