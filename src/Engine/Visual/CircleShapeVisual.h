#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>

class CircleShapeVisual : public ShapeVisualBase
{
public:
	CircleShapeVisual();
	sf::Shape* GetBaseShape() override;

	sf::CircleShape* GetShape();

private:
	sf::CircleShape _circle;
};
