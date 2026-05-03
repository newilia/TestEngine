#include "Engine/Visual/SpriteVisual.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "SpriteVisual.generated.hpp"

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
	auto node = GetNode();
	const sf::Transform nw = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfVisual(worldPoint, this, nw);
}

void SpriteVisual::SetTexture(const sf::Texture& texture) {
	_sprite = std::make_shared<sf::Sprite>(texture);
}

sf::Vector2f SpriteVisual::GetSize() const {
	if (!_sprite) {
		return {};
	}
	const sf::FloatRect b = _sprite->getGlobalBounds();
	return sf::Vector2f{b.size};
}

void SpriteVisual::SetSize(const sf::Vector2f& size) {
	if (!_sprite || size.x <= 0.f || size.y <= 0.f) {
		return;
	}
	const sf::Texture& tex = _sprite->getTexture();
	const sf::Vector2u tsu = tex.getSize();
	if (tsu.x == 0 || tsu.y == 0) {
		return;
	}
	const float tx = static_cast<float>(tsu.x);
	const float ty = static_cast<float>(tsu.y);
	_sprite->setScale({size.x / tx, size.y / ty});
}
