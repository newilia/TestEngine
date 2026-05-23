#include "Engine/Core/AssetRegistry.h"

#include <utility>
#include <vector>

namespace Engine {
	namespace {

		std::vector<AssetRegistration>& PendingAssetRegistrations() {
			static std::vector<AssetRegistration> pending;
			return pending;
		}

	} // namespace

	namespace AssetRegistrationDetail {

		void AppendAssetRegistration(AssetRegistration registration) {
			PendingAssetRegistrations().push_back(std::move(registration));
		}

	} // namespace AssetRegistrationDetail

	const AssetRegistry& AssetRegistry::GetInstance() {
		static const AssetRegistry instance;
		return instance;
	}

	AssetRegistry::AssetRegistry() {
		_registrations = PendingAssetRegistrations();
		for (const AssetRegistration& registration : _registrations) {
			_typeIdByType.emplace(registration.assetType, registration.typeId);
		}
	}

	const std::vector<AssetRegistration>& AssetRegistry::GetAll() const {
		return _registrations;
	}

	std::string_view AssetRegistry::GetTypeIdFor(std::type_index assetType) const {
		const auto it = _typeIdByType.find(assetType);
		if (it == _typeIdByType.end()) {
			return {};
		}
		return it->second;
	}

	std::shared_ptr<void> AssetRegistry::LoadByTypeId(
	    std::string_view typeId, const std::filesystem::path& absolutePath) const {
		for (const AssetRegistration& registration : _registrations) {
			if (registration.typeId == typeId && registration.load) {
				return registration.load(absolutePath);
			}
		}
		return nullptr;
	}

} // namespace Engine
