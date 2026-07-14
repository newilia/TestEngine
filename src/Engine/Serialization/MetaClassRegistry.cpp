#include "Engine/Serialization/MetaClassRegistry.h"

#include <utility>
#include <vector>

namespace Engine::Serialization {
	namespace {

		std::vector<MetaClassRegistration>& PendingMetaClassRegistrations() {
			static std::vector<MetaClassRegistration> pending;
			return pending;
		}

	} // namespace

	namespace MetaClassRegistrationDetail {

		void AppendMetaClassRegistration(MetaClassRegistration registration) {
			PendingMetaClassRegistrations().push_back(std::move(registration));
		}

	} // namespace MetaClassRegistrationDetail

	const MetaClassRegistry& MetaClassRegistry::GetInstance() {
		static const MetaClassRegistry instance;
		return instance;
	}

	MetaClassRegistry::MetaClassRegistry() {
		_registrations = PendingMetaClassRegistrations();
	}

	const std::vector<MetaClassRegistration>& MetaClassRegistry::GetAll() const {
		return _registrations;
	}

	std::unique_ptr<IPropertiesProvider> MetaClassRegistry::CreateUniqueByTypeId(std::string_view typeId) const {
		for (const MetaClassRegistration& registration : _registrations) {
			if (registration.typeId == typeId && registration.createInstance) {
				return registration.createInstance();
			}
		}
		return nullptr;
	}

	std::string MetaClassRegistry::GetTypeIdForInstance(const IPropertiesProvider& instance) const {
		for (const MetaClassRegistration& registration : _registrations) {
			if (registration.isType && registration.isType(&instance)) {
				return registration.typeId;
			}
		}
		return {};
	}

	std::vector<std::string> MetaClassRegistry::GetDerivedTypeIds(std::string_view baseClassShortName) const {
		std::vector<std::string> result;
		std::vector<std::string> frontier = {std::string(baseClassShortName)};
		while (!frontier.empty()) {
			const std::string baseName = std::move(frontier.back());
			frontier.pop_back();
			for (const MetaClassRegistration& registration : _registrations) {
				if (registration.propertyBaseShortName != baseName) {
					continue;
				}
				result.push_back(registration.typeId);
				frontier.push_back(registration.classShortName);
			}
		}
		return result;
	}

} // namespace Engine::Serialization
