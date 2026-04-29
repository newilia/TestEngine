#include "TextVisual.h"

#include "Engine/App/Utils.h"

#include <SFML/Graphics/Text.hpp>

TextVisual::TextVisual(std::shared_ptr<sf::Text> text) : _text(std::move(text)) {}

const sf::Text* TextVisual::GetText() const {
	return _text.get();
}

void TextVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_text) {
		target.draw(*_text, states);
	}
}

bool TextVisual::HitTest(sf::Vector2f windowPosition) const {
	return Utils::IsPointInsideOfVisual(windowPosition, this);
}
