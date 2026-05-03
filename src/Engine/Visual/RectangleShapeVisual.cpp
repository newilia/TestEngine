#include "Engine/Visual/RectangleShapeVisual.h"

#include "Engine/Core/Utils.h"

RectangleShapeVisual::RectangleShapeVisual() {}

sf::Shape* RectangleShapeVisual::GetBaseShape() {
	return &_rect;
}

bool RectangleShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return Utils::IsWorldPointInsideOfShape(worldPoint, &_rect);
}

sf::RectangleShape* RectangleShapeVisual::GetShape() {
	return &_rect;
}
