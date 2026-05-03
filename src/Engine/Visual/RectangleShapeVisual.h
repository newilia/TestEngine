#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RectangleShape.hpp>

class RectangleShapeVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	RectangleShapeVisual();
	sf::Shape* GetBaseShape() override;

	sf::RectangleShape* GetShape();

	/// @getter
	sf::Vector2f GetRectSize() const;
	/// @setter
	void SetRectSize(const sf::Vector2f& size);

private:
	sf::RectangleShape _rect;
};
