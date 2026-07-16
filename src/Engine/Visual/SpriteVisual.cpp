#include "Engine/Visual/SpriteVisual.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/ShapeHitTestUtils.h"
#include "Engine/Core/TextureManager.h"
#include "SpriteVisual.generated.hpp"

#include <SFML/Graphics/Sprite.hpp>

#include <filesystem>

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

const sf::Transform* SpriteVisual::GetTransform() const {
	return _sprite ? &_sprite->getTransform() : nullptr;
}

sf::FloatRect SpriteVisual::GetLocalBounds() const {
	if (_sprite) {
		return _sprite->getLocalBounds();
	}
	return {};
}

sf::FloatRect SpriteVisual::GetGlobalBounds() const {
	if (_sprite) {
		return _sprite->getGlobalBounds();
	}
	return {};
}

void SpriteVisual::SetTexture(const sf::Texture& texture) {
	_sprite = std::make_unique<sf::Sprite>(texture);
}

const std::string& SpriteVisual::GetTexturePath() const {
	return _texturePath;
}

void SpriteVisual::SetTexturePath(std::string path) {
	_texturePath = std::move(path);
	_sprite.reset();

	if (_texturePath.empty()) {
		return;
	}

	auto tm = Engine::MainContext::GetInstance().GetTextureManager();
	if (!tm) {
		return;
	}

	const std::shared_ptr<sf::Texture> texture = tm->LoadTexture(std::filesystem::path(_texturePath));
	if (texture) {
		SetTexture(*texture);
		if (_sprite && _textureRect) {
			_sprite->setTextureRect(*_textureRect);
		}
	}
}

void SpriteVisual::SetOrigin(const sf::Vector2f& origin) {
	if (_sprite) {
		_sprite->setOrigin(origin);
	}
}

sf::Vector2f SpriteVisual::GetOrigin() const {
	return _sprite ? _sprite->getOrigin() : sf::Vector2f{};
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

std::optional<sf::IntRect> SpriteVisual::GetTextureRect() const {
	return _textureRect;
}

void SpriteVisual::SetTextureRect(std::optional<sf::IntRect> textureRect) {
	_textureRect = textureRect;
	if (!_sprite) {
		return;
	}
	if (_textureRect) {
		_sprite->setTextureRect(*_textureRect);
		return;
	}
	const sf::Vector2u sz = _sprite->getTexture().getSize();
	if (sz.x > 0 && sz.y > 0) {
		_sprite->setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(static_cast<int>(sz.x), static_cast<int>(sz.y))));
	}
}

sf::Color SpriteVisual::GetColor() const {
	if (!_sprite) {
		return {};
	}
	return _sprite->getColor();
}

void SpriteVisual::SetColor(const sf::Color& color) {
	if (!_sprite) {
		return;
	}
	_sprite->setColor(color);
}
