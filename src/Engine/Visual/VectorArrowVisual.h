#pragma once

#include "Visual.h"
#include "VectorArrow.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

class VectorArrowVisual : public Visual
{
public:
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	bool HitTest(sf::Vector2f windowPosition) const override;

	void SetStartPos(const sf::Vector2f& start);

	void SetEndPos(const sf::Vector2f& end);

	void SetColor(const sf::Color& color);

	void SetVisible(bool visible);

private:
	VectorArrow _arrow;
	bool _visible = false;
};
