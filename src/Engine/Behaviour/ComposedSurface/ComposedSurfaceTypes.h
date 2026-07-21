#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf {
	class Texture;
}

namespace Engine {

	enum class ComposedSurfaceContributorKind
	{
		Tile,
		SphereProjection
	};

	struct ComposedSurfaceTileData
	{
		const sf::Texture* texture = nullptr;
		sf::Vector2f tiling{2.f, 2.f};
		sf::Vector2f uvOffset{};
		sf::Color tint = sf::Color::White;
	};

	struct ComposedSurfaceSphereProjectionData
	{
		bool active = false;
		sf::Vector2f sphereUvOffset{};
	};

} // namespace Engine
