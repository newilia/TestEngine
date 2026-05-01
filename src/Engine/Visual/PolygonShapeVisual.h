#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

/// Любой другой подкласс `sf::Shape`: веер по вершинам (как у тела).
class PolygonShapeVisual : public ShapeVisualBase
{
public:
	explicit PolygonShapeVisual(sf::Shape* shape);
	bool HitTest(const sf::Vector2f& worldPoint) const override;
};
