#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>

class CircleShapeVisual : public ShapeVisualBase
{
public:
	explicit CircleShapeVisual(sf::CircleShape* circle);
	bool HitTest(const sf::Vector2f& worldPoint) const override;

private:
	sf::CircleShape* _circle = nullptr;
};
