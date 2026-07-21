#pragma once

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <string>

namespace sf {
	class Texture;
}

namespace Engine {

	class TiledTextureContributorBehaviour : public ComposedSurfaceContributorBehaviour
	{
		META_CLASS()

	public:
		[[nodiscard]] bool IsContributorEnabled() const override;
		[[nodiscard]] ComposedSurfaceContributorKind GetContributorKind() const override;
		[[nodiscard]] bool TryContributeTile(ComposedSurfaceTileData& out) const override;

		[[nodiscard]] const std::string& GetTexturePath() const;
		void SetTexturePath(std::string path);

		[[nodiscard]] sf::Vector2f GetTiling() const;
		void SetTiling(sf::Vector2f value);

		[[nodiscard]] sf::Vector2f GetUvOffset() const;
		void SetUvOffset(sf::Vector2f value);

		[[nodiscard]] sf::Color GetTint() const;
		void SetTint(sf::Color value);

	private:
		void EnsureTextureLoaded() const;

		/// @property
		bool _isEnabled = true;
		/// @property
		/// @valuesProvider(GetTextures)
		std::string _texturePath;
		/// @property(dragSpeed=0.01f, minValue=0.01f)
		sf::Vector2f _tiling{2.f, 2.f};
		/// @property(dragSpeed=0.001f)
		sf::Vector2f _uvOffset{};
		/// @property
		sf::Color _tint = sf::Color::White;

		mutable std::shared_ptr<sf::Texture> _texture;
		mutable std::string _loadedTexturePath;
	};

} // namespace Engine
