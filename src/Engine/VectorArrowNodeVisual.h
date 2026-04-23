#pragma once

#include "NodeVisual.h"
#include "VectorArrow.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

class VectorArrowNodeVisual : public NodeVisual
{
public:
	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void setStartPos(const sf::Vector2f& start);

	void setEndPos(const sf::Vector2f& end);

	void setColor(const sf::Color& color);

	void setVisible(bool visible);

private:
	VectorArrow _arrow;
	bool _visible = false;
};
