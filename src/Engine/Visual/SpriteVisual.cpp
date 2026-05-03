#include "Engine/Visual/SpriteVisual.h"

#include "Engine/Core/Utils.h"

#include <SFML/Graphics/Sprite.hpp>

SpriteVisual::SpriteVisual() {}

const sf::Sprite* SpriteVisual::GetSprite() const {
	return _sprite.get();
}

void SpriteVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_sprite) {
		target.draw(*_sprite, states);
	}
}

bool SpriteVisual::HitTest(const sf::Vector2f& worldPoint) const {
	return Utils::IsWorldPointInsideOfVisual(worldPoint, this);
}

void SpriteVisual::SetTexture(const sf::Texture& texture) {
	_sprite = std::make_shared<sf::Sprite>(texture);
}
