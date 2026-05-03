#pragma once

#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/Shape.hpp>

class ShapeVisualBase : public Visual
{
public:
	ShapeVisualBase();
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	virtual sf::Shape* GetBaseShape() = 0;
	const sf::Shape* GetBaseShape() const;
};
