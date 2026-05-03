#include "TextVisual.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

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

bool TextVisual::HitTest(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	const sf::Transform nw = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfVisual(worldPoint, this, nw);
}
