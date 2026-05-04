#pragma once

#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/Shape.hpp>

#include <Engine/Core/MetaClass.h>

class ShapeVisualBase : public Visual
{
	META_CLASS()
	META_PROPERTY_BASE(Visual)

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
	/// @getter
	sf::Vector2f GetPosition() const;
	/// @setter
	void SetPosition(const sf::Vector2f& position);
	/// @getter
	sf::Angle GetRotation() const;
	/// @setter
	void SetRotation(sf::Angle angle);
	/// @getter
	sf::Vector2f GetScale() const;
	/// @setter
	void SetScale(const sf::Vector2f& scale);
	/// @getter
	sf::Vector2f GetOrigin() const;
	/// @setter
	void SetOrigin(const sf::Vector2f& origin);
	/// @getter
	float GetMiterLimit() const;
	/// @setter
	void SetMiterLimit(float miterLimit);
};
