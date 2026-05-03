#include "Engine/Visual/CircleShapeVisual.h"

#include "Engine/Core/Utils.h"

CircleShapeVisual::CircleShapeVisual() {}

sf::Shape* CircleShapeVisual::GetBaseShape() {
	return &_circle;
}

sf::CircleShape* CircleShapeVisual::GetShape() {
	return &_circle;
}

bool CircleShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return Utils::IsWorldPointInsideOfShape(worldPoint, &_circle);
}
