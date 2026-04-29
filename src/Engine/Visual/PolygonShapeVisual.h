#pragma once

#include "Engine/Visual/ShapeVisualBase.h"

/// Любой другой подкласс `sf::Shape`: веер по вершинам (как у тела).
class PolygonShapeVisual : public ShapeVisualBase
{
public:
	explicit PolygonShapeVisual(sf::Shape* shape);
	bool HitTest(sf::Vector2f windowPosition) const override;
};
