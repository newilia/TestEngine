#pragma once

#include "Engine/Core/MetaClass.h"
#include "Engine/Visual/Visual.h"

#include <memory>
#include <vector>

namespace sf {
	class Sprite;
}

class SpriteVisual : public Visual
{
	META_CLASS()

public:
	SpriteVisual();

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	const sf::Sprite* GetSprite() const;
	void SetTexture(const sf::Texture& texture);
	/// @getter(name="Size")
	sf::Vector2f GetSize() const;
	/// @setter(name="Size")
	void SetSize(const sf::Vector2f& size);

private:
	std::shared_ptr<sf::Sprite> _sprite;
};
