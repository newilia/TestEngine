#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>

class CircleShapeVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	CircleShapeVisual() = default;
	const sf::Shape* GetBaseShape() const override;

public:
	/// @getter
	float GetRadius() const;
	/// @setter
	void SetRadius(float radius);
	/// @getter
	int GetPointCount() const;
	/// @setter
	void SetPointCount(int count);
	/// @getter
	sf::Vector2f GetOrigin() const;
	/// @setter
	void SetOrigin(const sf::Vector2f& origin);

private:
	sf::CircleShape _circle;
};
