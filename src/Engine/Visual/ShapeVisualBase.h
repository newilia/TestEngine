#pragma once

#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/Shape.hpp>

#include <Engine/Core/MetaClass.h>

class ShapeVisualBase : public Visual
{
	META_CLASS()

public:
	ShapeVisualBase();
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	virtual sf::Shape* GetBaseShape() = 0;
	const sf::Shape* GetBaseShape() const;

public:
	/// @getter(name="FillColor")
	sf::Color GetFillColor();
	/// @setter(name="FillColor")
	void SetFillColor(const sf::Color& color);
	/// @getter(name="OutlineColor")
	sf::Color GetOutlineColor();
	/// @setter(name="OutlineColor")
	void SetOutlineColor(const sf::Color& color);
	/// @getter(name="OutlineThickness")
	float GetOutlineThickness();
	/// @setter(name="OutlineThickness")
	void SetOutlineThickness(float thickness);
};
