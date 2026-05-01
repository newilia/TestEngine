#include "Engine/Visual/RectangleShapeVisual.h"

#include "Engine/App/Utils.h"

RectangleShapeVisual::RectangleShapeVisual(sf::RectangleShape* rect) : ShapeVisualBase(rect), _rect(rect) {}

bool RectangleShapeVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return _rect && Utils::IsWorldPointInsideOfShape(worldPoint, _rect);
}
