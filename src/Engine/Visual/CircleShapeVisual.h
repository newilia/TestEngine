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
	sf::Shape* GetBaseShape() override;

	sf::CircleShape* GetShape();

public:
	/// @getter
	float GetRadius() const;
	/// @setter
	void SetRadius(float radius);
	/// @getter
	int GetPointCount() const;
	/// @setter
	void SetPointCount(int count);

private:
	sf::CircleShape _circle;
};
