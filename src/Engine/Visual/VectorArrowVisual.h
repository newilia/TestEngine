#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/Visual.h"
#include "SFML/Graphics.hpp"
#include "VectorArrow.h"

class VectorArrowVisual : public Visual
{
	META_CLASS()
	META_PROPERTY_BASE(Visual)

public:
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;

public:
	/// @getter
	sf::Vector2f GetStartPos() const;
	/// @getter
	sf::Vector2f GetEndPos() const;
	/// @getter
	sf::Color GetColor() const;
	/// @getter
	float GetArrowHeadAngle() const;
	/// @getter
	float GetArrowHeadSize() const;

	/// @setter
	void SetStartPos(const sf::Vector2f& start);
	/// @setter
	void SetEndPos(const sf::Vector2f& end);
	/// @setter
	void SetColor(const sf::Color& color);
	/// @setter
	void SetArrowHeadAngle(float angle);
	/// @setter
	void SetArrowHeadSize(float size);

private:
	VectorArrow _arrow;
};
