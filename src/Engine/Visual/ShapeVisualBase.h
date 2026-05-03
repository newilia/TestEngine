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
	/// @getter
	sf::Color GetFillColor();
	/// @setter
	void SetFillColor(const sf::Color& color);
	/// @getter
	sf::Color GetOutlineColor();
	/// @setter
	void SetOutlineColor(const sf::Color& color);
	/// @getter
	float GetOutlineThickness();
	/// @setter
	void SetOutlineThickness(float thickness);
};
