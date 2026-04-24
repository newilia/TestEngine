#include "Engine/Visual/RectangleShapeVisual.h"

#include "Engine/App/Utils.h"

RectangleShapeVisual::RectangleShapeVisual(sf::RectangleShape* rect) : ShapeVisualBase(rect), _rect(rect) {}

bool RectangleShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _rect && utils::IsPointInsideOfShape(windowPosition, _rect);
}
