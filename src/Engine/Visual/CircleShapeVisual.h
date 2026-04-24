#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>

class CircleShapeVisual : public ShapeVisualBase
{
public:
	explicit CircleShapeVisual(sf::CircleShape* circle);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::CircleShape* _circle = nullptr;
};
