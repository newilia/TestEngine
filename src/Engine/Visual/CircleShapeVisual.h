#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/CircleShape.h"
#include "Engine/Visual/ShapeVisualBase.h"

class CircleShapeVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	CircleShapeVisual();
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
	bool GetDrawSector() const;
	/// @setter
	void SetDrawSector(bool drawSector);
	/// @getter
	sf::Color GetSectorColor() const;
	/// @setter
	void SetSectorColor(sf::Color color);

private:
	CircleShape _circle;
};
