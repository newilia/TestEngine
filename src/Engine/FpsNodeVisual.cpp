#include "FpsNodeVisual.h"

#include <SFML/Graphics/Text.hpp>

void FpsNodeVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}
