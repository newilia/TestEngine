#include "Engine/Core/AssetManager.h"

#include "Engine/Core/AssetRegistry.h"

namespace Engine {

	void AssetManager::Invalidate(const std::filesystem::path& relativePath) {
		if (relativePath.empty()) {
			return;
		}
		const std::string pathKey = relativePath.generic_string();
		std::lock_guard lock(_mutex);
		for (auto it = _cache.begin(); it != _cache.end();) {
			const std::string_view key = it->first;
			const std::size_t sep = key.find('|');
			if (sep != std::string::npos && key.substr(sep + 1) == pathKey) {
				it = _cache.erase(it);
			}
			else {
				++it;
			}
		}
	}

	void AssetManager::InvalidateAll() {
		std::lock_guard lock(_mutex);
		_cache.clear();
	}

} // namespace Engine
