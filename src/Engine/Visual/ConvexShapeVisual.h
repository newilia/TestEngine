#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/ConvexShape.hpp>

#include <vector>

class ConvexShapeVisual : public ShapeVisualBase
{
	META_CLASS()

public:
	ConvexShapeVisual();
	sf::Shape* GetBaseShape() override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;

	sf::ConvexShape* GetShape();
	/// @getter(name="Points")
	std::vector<sf::Vector2f> GetPoints() const;
	/// @setter(name="Points")
	void SetPoints(const std::vector<sf::Vector2f>& points);

private:
	sf::ConvexShape _convex;
};
