#pragma once

#include "SFML/Graphics.hpp"

class Visual;

namespace Utils {
	/// Веерная триангуляция от первой вершины; Не для круга — см. `IsWorldPointInsideOfShape`.
	bool IsWorldPointInsideOfShapeByFan(const sf::Vector2f& worldPoint, const sf::Shape* shape);
	bool IsWorldPointInsideOfShapeByFan(
	    const sf::Vector2f& worldPoint, const sf::Shape* shape, const sf::Transform& nodeWorld);
	bool IsWorldPointInsideOfShape(const sf::Vector2f& worldPoint, const sf::Shape* shape);
	bool IsWorldPointInsideOfShape(
	    const sf::Vector2f& worldPoint, const sf::Shape* shape, const sf::Transform& nodeWorld);
	bool IsWorldPointInsideOfVisual(const sf::Vector2f& worldPoint, const Visual* visual);
	bool IsWorldPointInsideOfVisual(
	    const sf::Vector2f& worldPoint, const Visual* visual, const sf::Transform& nodeWorld);
} // namespace Utils
