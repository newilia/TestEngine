#include "VectorArrowNodeVisual.h"

#include <SFML/Graphics/RenderTarget.hpp>

void VectorArrowNodeVisual::setStartPos(const sf::Vector2f& start) {
	_arrow.setStartPos(start);
}

void VectorArrowNodeVisual::setEndPos(const sf::Vector2f& end) {
	_arrow.setEndPos(end);
}

void VectorArrowNodeVisual::setColor(const sf::Color& color) {
	_arrow.setColor(color);
}

void VectorArrowNodeVisual::setVisible(bool visible) {
	_visible = visible;
}

void VectorArrowNodeVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_visible) {
		return;
	}
	target.draw(_arrow, states);
}
