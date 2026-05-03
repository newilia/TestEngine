#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class TextureManager
{
public:
	TextureManager() = default;

	/// Returns a cached texture for this path, or loads from disk. On failure returns nullptr.
	std::shared_ptr<sf::Texture> LoadTexture(const std::filesystem::path& path);

private:
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> _textures;
};
