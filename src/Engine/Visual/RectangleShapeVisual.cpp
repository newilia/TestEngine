#include "Engine/Visual/RectangleShapeVisual.h"

#include "RectangleShapeVisual.generated.hpp"

RectangleShapeVisual::RectangleShapeVisual() {}

sf::Shape* RectangleShapeVisual::GetBaseShape() {
	return &_rect;
}

sf::RectangleShape* RectangleShapeVisual::GetShape() {
	return &_rect;
}

sf::Vector2f RectangleShapeVisual::GetRectSize() const {
	return _rect.getSize();
}

void RectangleShapeVisual::SetRectSize(const sf::Vector2f& size) {
	_rect.setSize(size);
}
