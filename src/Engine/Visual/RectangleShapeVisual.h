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
	const sf::Shape* GetBaseShape() const override;

	sf::RectangleShape* GetShape();

	/// @getter
	sf::Vector2f GetSize() const;
	/// @setter
	void SetSize(const sf::Vector2f& size);

private:
	sf::RectangleShape _rect;
};
