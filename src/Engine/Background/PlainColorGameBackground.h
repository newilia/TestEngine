#pragma once

#include "Engine/Background/IGameBackground.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>

namespace sf {
	class RenderWindow;
}

/// Fills the current camera view rectangle with a solid color (world-space quad).
class PlainColorGameBackground final : public IGameBackground
{
	META_CLASS()

public:
	void Update(const sf::RenderWindow& window, sf::Time dt) override;

	void SetFillColor(const sf::Color& color);
	const sf::Color& GetFillColor() const;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	/// @property
	sf::Color _fillColor = sf::Color(32, 32, 48);
};
