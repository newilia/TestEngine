#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>

class CircleShapeVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	CircleShapeVisual();
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
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
	/// @getter(name="DrawSector");
	bool IsDrawSector() const;
	/// @setter(name="DrawSector");
	void SetDrawSector(bool drawSector);
	/// @getter
	sf::Color GetSectorColor() const;
	/// @setter
	void SetSectorColor(sf::Color color);

private:
	void RebuildSectorVertices() const;

	sf::CircleShape _circle;
	bool _drawSector = true;
	sf::Color _sectorColor;
	mutable sf::VertexArray _sectorVertices;
	mutable bool _isSectorDirty = true;
};
