#include "Engine/Visual/MakeShapeVisual.h"

#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/ConvexShapeVisual.h"
#include "Engine/Visual/PolygonShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shape.hpp>

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
