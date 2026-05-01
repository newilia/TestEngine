#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RectangleShape.hpp>

class RectangleShapeVisual : public ShapeVisualBase
{
public:
	explicit RectangleShapeVisual(sf::RectangleShape* rect);
	bool HitTest(const sf::Vector2f& worldPoint) const override;

private:
	sf::RectangleShape* _rect = nullptr;
};
