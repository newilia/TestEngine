#include "TextureManager.h"

#include <iostream>

std::shared_ptr<sf::Texture> TextureManager::LoadTexture(const std::filesystem::path& path) {
	const std::string key = path.generic_string();
	if (const auto it = _textures.find(key); it != _textures.end()) {
		return it->second;
	}
	auto texture = std::make_shared<sf::Texture>();
	if (!texture->loadFromFile(path)) {
		std::cerr << "TextureManager: failed to load texture: " << key << '\n';
		return nullptr;
	}
	_textures.emplace(key, texture);
	return texture;
}
