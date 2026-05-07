#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/Visual.h"
#include "SFML/Graphics.hpp"
#include "VectorArrowShape.h"

class VectorArrowVisual : public ShapeVisualBase
{
	META_CLASS()
	META_PROPERTY_BASE(ShapeVisualBase)

public:
	VectorArrowVisual() = default;

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	const sf::Shape* GetBaseShape() const override;
	sf::FloatRect GetLocalBounds() const override;

public:
	/// @getter
	sf::Vector2f GetStartPos() const;
	/// @getter
	sf::Vector2f GetEndPos() const;
	/// @getter
	sf::Color GetFillColor() const;
	/// @getter
	sf::Angle GetArrowHeadAngle() const;
	/// @getter
	float GetArrowHeadSize() const;

	/// @setter
	void SetStartPos(const sf::Vector2f& start);
	/// @setter
	void SetEndPos(const sf::Vector2f& end);
	/// @setter
	void SetFillColor(const sf::Color& color);
	/// @setter
	void SetArrowHeadAngle(sf::Angle angle);
	/// @setter
	void SetArrowHeadSize(float size);

private:
	VectorArrowShape _arrowShape;
};
