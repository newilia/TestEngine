#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {

	struct AssetRegistration
	{
		std::string typeId;
		std::type_index assetType{typeid(void)};
		std::function<std::shared_ptr<void>(const std::filesystem::path& absolutePath)> load;
	};

	class AssetRegistry
	{
	public:
		static const AssetRegistry& GetInstance();

		const std::vector<AssetRegistration>& GetAll() const;

		[[nodiscard]] std::string_view GetTypeIdFor(std::type_index assetType) const;
		[[nodiscard]] std::shared_ptr<void> LoadByTypeId(
		    std::string_view typeId, const std::filesystem::path& absolutePath) const;

		template <typename TAsset>
		[[nodiscard]] std::shared_ptr<TAsset> Load(const std::filesystem::path& absolutePath) const {
			const std::string_view typeId = GetTypeIdFor(std::type_index(typeid(TAsset)));
			if (typeId.empty()) {
				return nullptr;
			}
			if (auto loaded = LoadByTypeId(typeId, absolutePath)) {
				return std::static_pointer_cast<TAsset>(loaded);
			}
			return nullptr;
		}

	private:
		AssetRegistry();

		std::vector<AssetRegistration> _registrations;
		std::unordered_map<std::type_index, std::string_view> _typeIdByType;
	};

} // namespace Engine
