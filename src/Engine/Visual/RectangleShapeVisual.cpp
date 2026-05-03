#include "Engine/Visual/RectangleShapeVisual.h"

RectangleShapeVisual::RectangleShapeVisual() {}

sf::Shape* RectangleShapeVisual::GetBaseShape() {
	return &_rect;
}

sf::RectangleShape* RectangleShapeVisual::GetShape() {
	return &_rect;
}
