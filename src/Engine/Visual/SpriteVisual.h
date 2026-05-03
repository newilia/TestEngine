#pragma once

#include "Engine/Visual/Visual.h"

#include <memory>

namespace sf {
	class Sprite;
}

class SpriteVisual : public Visual
{
public:
	SpriteVisual();

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	void SetTexture(const sf::Texture& texture);
	void SetSize(const sf::Vector2f& size);
	const sf::Sprite* GetSprite() const;

private:
	std::shared_ptr<sf::Sprite> _sprite;
};
