#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RectangleShape.hpp>

class RectangleShapeVisual : public ShapeVisualBase
{
public:
	explicit RectangleShapeVisual(sf::RectangleShape* rect);
	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::RectangleShape* _rect = nullptr;
};
