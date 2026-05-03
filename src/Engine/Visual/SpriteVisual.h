#pragma once

#include "Engine/Visual/Visual.h"

#include <memory>

namespace sf {
	class Sprite;
}

class SpriteVisual : public Visual
{
public:
	explicit SpriteVisual(std::shared_ptr<sf::Sprite> sprite);

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool HitTest(const sf::Vector2f& worldPoint) const override;
	const sf::Sprite* GetSprite() const;

private:
	std::shared_ptr<sf::Sprite> _sprite;
};
