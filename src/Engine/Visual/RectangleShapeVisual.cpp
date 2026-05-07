#include "Engine/Visual/RectangleShapeVisual.h"

#include "RectangleShapeVisual.generated.hpp"

RectangleShapeVisual::RectangleShapeVisual() {}

const sf::Shape* RectangleShapeVisual::GetBaseShape() const {
	return &_rect;
}

sf::RectangleShape* RectangleShapeVisual::GetShape() {
	return &_rect;
}

sf::Vector2f RectangleShapeVisual::GetSize() const {
	return _rect.getSize();
}

void RectangleShapeVisual::SetSize(const sf::Vector2f& size) {
	_rect.setSize(size);
}
