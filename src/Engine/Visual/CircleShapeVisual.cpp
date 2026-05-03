#include "Engine/Visual/CircleShapeVisual.h"

CircleShapeVisual::CircleShapeVisual() {}

sf::Shape* CircleShapeVisual::GetBaseShape() {
	return &_circle;
}

sf::CircleShape* CircleShapeVisual::GetShape() {
	return &_circle;
}
