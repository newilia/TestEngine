#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/Visual.h"

namespace sf {
	class Sprite;
}

class SpriteVisual : public Visual
{
	META_CLASS()
	META_PROPERTY_BASE(Visual)

public:
	SpriteVisual() = default;

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	sf::FloatRect GetLocalBounds() const override;
	sf::FloatRect GetGlobalBounds() const override;

	const sf::Transform* GetTransform() const override;

	void SetOrigin(const sf::Vector2f& origin);
	sf::Vector2f GetOrigin() const;

	const sf::Sprite* GetSprite() const;
	void SetTexture(const sf::Texture& texture);

public:
	/// @getter
	/// @valuesProvider(GetTextures)
	const std::string& GetTexturePath() const;
	/// @setter
	void SetTexturePath(std::string path);

	/// @getter
	sf::Vector2f GetSize() const;
	/// @setter
	void SetSize(const sf::Vector2f& size);

	/// @getter
	std::optional<sf::IntRect> GetTextureRect() const;
	/// @setter
	void SetTextureRect(std::optional<sf::IntRect> textureRect);

	/// @getter
	sf::Color GetColor() const;
	/// @setter
	void SetColor(const sf::Color& opacity);

private:
	std::string _texturePath;
	std::unique_ptr<sf::Sprite> _sprite;
	std::optional<sf::IntRect> _textureRect;
	sf::Color _color;
};
