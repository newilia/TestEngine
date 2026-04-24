#include "ShapeVisual.h"

#include "Engine/App/Utils.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (GetShape()) {
		target.draw(*GetShape(), states);
	}
}

CircleShapeVisual::CircleShapeVisual(sf::CircleShape* circle) : ShapeVisualBase(circle), _circle(circle) {}

bool CircleShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _circle && utils::IsPointInsideOfShape(windowPosition, _circle);
}

RectangleShapeVisual::RectangleShapeVisual(sf::RectangleShape* rect) : ShapeVisualBase(rect), _rect(rect) {}

bool RectangleShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _rect && utils::IsPointInsideOfShape(windowPosition, _rect);
}

ConvexShapeVisual::ConvexShapeVisual(sf::ConvexShape* convex) : ShapeVisualBase(convex), _convex(convex) {}

bool ConvexShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return _convex && utils::IsPointInsideOfShape(windowPosition, _convex);
}

PolygonShapeVisual::PolygonShapeVisual(sf::Shape* shape) : ShapeVisualBase(shape) {}

bool PolygonShapeVisual::HitTest(sf::Vector2f windowPosition) const {
	return utils::IsPointInsideOfShape(windowPosition, GetShape());
}

std::shared_ptr<Visual> MakeShapeVisual(sf::Shape* shape) {
	if (!shape) {
		return nullptr;
	}
	if (auto* c = dynamic_cast<sf::CircleShape*>(shape)) {
		return std::make_shared<CircleShapeVisual>(c);
	}
	if (auto* r = dynamic_cast<sf::RectangleShape*>(shape)) {
		return std::make_shared<RectangleShapeVisual>(r);
	}
	if (auto* x = dynamic_cast<sf::ConvexShape*>(shape)) {
		return std::make_shared<ConvexShapeVisual>(x);
	}
	return std::make_shared<PolygonShapeVisual>(shape);
}
