#pragma once

#include "Engine/Core/AssetRegistry.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace Engine {

	namespace AssetRegistrationDetail {
		void AppendAssetRegistration(AssetRegistration registration);
	}

	template <typename TAsset>
	void RegisterAsset(std::string_view typeId,
	    std::function<std::shared_ptr<TAsset>(const std::filesystem::path& absolutePath)> load) {
		AssetRegistration registration;
		registration.typeId = std::string(typeId);
		registration.assetType = std::type_index(typeid(TAsset));
		registration.load = [load = std::move(load)](
		                        const std::filesystem::path& absolutePath) -> std::shared_ptr<void> {
			return load(absolutePath);
		};
		AssetRegistrationDetail::AppendAssetRegistration(std::move(registration));
	}

} // namespace Engine
