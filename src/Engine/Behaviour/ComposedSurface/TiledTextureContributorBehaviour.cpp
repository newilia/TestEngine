#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/TextureManager.h"
#include "TiledTextureContributorBehaviour.generated.hpp"

#include <SFML/Graphics/Texture.hpp>

#include <filesystem>

namespace Engine {

	bool TiledTextureContributorBehaviour::IsContributorEnabled() const {
		return _isEnabled;
	}

	ComposedSurfaceContributorKind TiledTextureContributorBehaviour::GetContributorKind() const {
		return ComposedSurfaceContributorKind::Tile;
	}

	bool TiledTextureContributorBehaviour::TryContributeTile(ComposedSurfaceTileData& out) const {
		if (!_isEnabled) {
			return false;
		}
		EnsureTextureLoaded();
		if (!_texture) {
			return false;
		}
		out.texture = _texture.get();
		out.tiling = _tiling;
		out.uvOffset = _uvOffset;
		out.tint = _tint;
		return true;
	}

	const std::string& TiledTextureContributorBehaviour::GetTexturePath() const {
		return _texturePath;
	}

	void TiledTextureContributorBehaviour::SetTexturePath(std::string path) {
		_texturePath = std::move(path);
		_texture.reset();
		_loadedTexturePath.clear();
	}

	sf::Vector2f TiledTextureContributorBehaviour::GetTiling() const {
		return _tiling;
	}

	void TiledTextureContributorBehaviour::SetTiling(sf::Vector2f value) {
		_tiling = {std::max(value.x, 0.01f), std::max(value.y, 0.01f)};
	}

	sf::Vector2f TiledTextureContributorBehaviour::GetUvOffset() const {
		return _uvOffset;
	}

	void TiledTextureContributorBehaviour::SetUvOffset(sf::Vector2f value) {
		_uvOffset = value;
	}

	sf::Color TiledTextureContributorBehaviour::GetTint() const {
		return _tint;
	}

	void TiledTextureContributorBehaviour::SetTint(sf::Color value) {
		_tint = value;
	}

	void TiledTextureContributorBehaviour::EnsureTextureLoaded() const {
		if (_texturePath.empty()) {
			_texture.reset();
			_loadedTexturePath.clear();
			return;
		}
		if (_texture && _loadedTexturePath == _texturePath) {
			return;
		}
		auto tm = MainContext::GetInstance().GetTextureManager();
		if (!tm) {
			return;
		}
		_texture = tm->LoadTexture(std::filesystem::path(_texturePath));
		if (_texture) {
			_texture->setRepeated(true);
			_texture->setSmooth(true);
		}
		_loadedTexturePath = _texturePath;
	}

} // namespace Engine
