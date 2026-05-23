#pragma once

#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/ContentPaths.h"

namespace Engine {

	template <typename TAsset>
	std::shared_ptr<TAsset> AssetManager::LoadUncached(const std::filesystem::path& relativePath) const {
		const std::filesystem::path absolutePath = ResolveContentPath(relativePath);
		return AssetRegistry::GetInstance().Load<TAsset>(absolutePath);
	}

} // namespace Engine
