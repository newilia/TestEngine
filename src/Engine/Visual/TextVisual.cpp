#include "TextVisual.h"

#include "Engine/App/Utils.h"

#include <SFML/Graphics/Text.hpp>

void TextVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}

bool TextVisual::HitTest(sf::Vector2f windowPosition) const {
	return utils::IsPointInsideOfVisual(windowPosition, this);
}
