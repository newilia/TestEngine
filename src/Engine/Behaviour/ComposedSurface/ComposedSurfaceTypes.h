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
		int sphereUnwrap = 0;
		float sphereOrientationX = 0.f;
		float sphereOrientationY = 0.f;
		float sphereOrientationZ = 0.f;
		float sphereOrientationW = 1.f;
	};

} // namespace Engine
