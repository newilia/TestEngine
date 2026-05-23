#pragma once

#include "Engine/Background/IGameBackground.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>

#include <filesystem>
#include <memory>

namespace sf {
	class RenderWindow;
} // namespace sf

/// Owns the active game background drawable and exposes facade helpers for switching implementations.
class GameBackgroundContext
{
public:
	void Update(const sf::RenderWindow& window, sf::Time dt);

	[[nodiscard]] IGameBackground* GetBackground() const;

	void ClearBackground();

	void SetPlainColorBackground(const sf::Color& color);

	void SetTiledBackground(
	    const std::filesystem::path& texturePath, float opacity, float staticity, float defaultScale);

private:
	std::unique_ptr<IGameBackground> _background;
};
