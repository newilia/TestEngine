#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/ConvexShape.hpp>

#include <vector>

class ConvexShapeVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	ConvexShapeVisual();
	sf::Shape* GetBaseShape() override;

	sf::ConvexShape* GetShape();
	/// @getter
	std::vector<sf::Vector2f> GetPoints() const;
	/// @setter
	void SetPoints(const std::vector<sf::Vector2f>& points);

private:
	sf::ConvexShape _convex;
};
