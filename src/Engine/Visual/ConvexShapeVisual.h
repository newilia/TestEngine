#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/ConvexShape.hpp>

class ConvexShapeVisual : public ShapeVisualBase
{
public:
	explicit ConvexShapeVisual(sf::ConvexShape* convex);
	bool HitTest(const sf::Vector2f& worldPoint) const override;

private:
	sf::ConvexShape* _convex = nullptr;
};
