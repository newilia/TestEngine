#include "ShapeNodeVisual.h"

#include "Engine/Utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

void ShapeNodeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (GetShape()) {
		target.draw(*GetShape(), states);
	}
}

CircleShapeNodeVisual::CircleShapeNodeVisual(sf::CircleShape* circle) : ShapeNodeVisualBase(circle), _circle(circle) {}

bool CircleShapeNodeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _circle && utils::isPointInsideOfShape(windowPosition, _circle);
}

RectangleShapeNodeVisual::RectangleShapeNodeVisual(sf::RectangleShape* rect) : ShapeNodeVisualBase(rect), _rect(rect) {}

bool RectangleShapeNodeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _rect && utils::isPointInsideOfShape(windowPosition, _rect);
}

ConvexShapeNodeVisual::ConvexShapeNodeVisual(sf::ConvexShape* convex) : ShapeNodeVisualBase(convex), _convex(convex) {}

bool ConvexShapeNodeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _convex && utils::isPointInsideOfShape(windowPosition, _convex);
}

PolygonShapeNodeVisual::PolygonShapeNodeVisual(sf::Shape* shape) : ShapeNodeVisualBase(shape) {}

bool PolygonShapeNodeVisual::HitTest(sf::Vector2f windowPosition) const {
	return utils::isPointInsideOfShape(windowPosition, GetShape());
}

std::shared_ptr<NodeVisual> MakeShapeNodeVisual(sf::Shape* shape) {
	if (!shape) {
		return nullptr;
	}
	if (auto* c = dynamic_cast<sf::CircleShape*>(shape)) {
		return std::make_shared<CircleShapeNodeVisual>(c);
	}
	if (auto* r = dynamic_cast<sf::RectangleShape*>(shape)) {
		return std::make_shared<RectangleShapeNodeVisual>(r);
	}
	if (auto* x = dynamic_cast<sf::ConvexShape*>(shape)) {
		return std::make_shared<ConvexShapeNodeVisual>(x);
	}
	return std::make_shared<PolygonShapeNodeVisual>(shape);
}
