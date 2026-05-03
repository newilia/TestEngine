#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RectangleShape.hpp>

class RectangleShapeVisual : public ShapeVisualBase
{
public:
	RectangleShapeVisual();
	sf::Shape* GetBaseShape() override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;

	sf::RectangleShape* GetShape();

private:
	sf::RectangleShape _rect;
};
