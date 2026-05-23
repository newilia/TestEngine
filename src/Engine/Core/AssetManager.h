#pragma once

#include "Engine/Core/AssetRegistry.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Engine {

	class AssetManager
	{
	public:
		template <typename TAsset>
		[[nodiscard]] std::shared_ptr<TAsset> GetOrLoad(const std::filesystem::path& relativePath) {
			if (relativePath.empty()) {
				return nullptr;
			}
			const std::string cacheKey = MakeCacheKey<TAsset>(relativePath);
			{
				std::lock_guard lock(_mutex);
				const auto it = _cache.find(cacheKey);
				if (it != _cache.end()) {
					if (auto locked = it->second.lock()) {
						return std::static_pointer_cast<TAsset>(locked);
					}
				}
			}
			const std::shared_ptr<TAsset> loaded = LoadUncached<TAsset>(relativePath);
			if (loaded) {
				std::lock_guard lock(_mutex);
				_cache[cacheKey] = loaded;
			}
			return loaded;
		}

		void Invalidate(const std::filesystem::path& relativePath);
		void InvalidateAll();

	private:
		template <typename TAsset>
		[[nodiscard]] static std::string MakeCacheKey(const std::filesystem::path& relativePath) {
			const AssetRegistry& registry = AssetRegistry::GetInstance();
			const std::string_view typeId = registry.GetTypeIdFor(std::type_index(typeid(TAsset)));
			return std::string(typeId) + "|" + relativePath.generic_string();
		}

		template <typename TAsset>
		[[nodiscard]] std::shared_ptr<TAsset> LoadUncached(const std::filesystem::path& relativePath) const;

		std::unordered_map<std::string, std::weak_ptr<void>> _cache;
		mutable std::mutex _mutex;
	};

} // namespace Engine

#include "Engine/Core/AssetManager.inl"
