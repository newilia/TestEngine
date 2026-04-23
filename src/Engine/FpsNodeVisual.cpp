#include "FpsNodeVisual.h"

#include "Utils.h"

#include <SFML/Graphics/Text.hpp>

void FpsNodeVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}

bool FpsNodeVisual::HitTest(sf::Vector2f windowPosition) const {
	return utils::isPointInsideOfNodeVisual(windowPosition, this);
}
