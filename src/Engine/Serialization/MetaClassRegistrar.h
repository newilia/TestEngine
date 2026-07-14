#pragma once

#include "Engine/Serialization/MetaClassRegistry.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace Engine::Serialization {

	namespace MetaClassRegistrationDetail {
		void AppendMetaClassRegistration(MetaClassRegistration registration);
	}

	template <typename TMetaClass>
	void RegisterMetaClass(
	    std::string_view typeId, std::string_view classShortName, std::string_view propertyBaseShortName) {
		MetaClassRegistration registration;
		registration.typeId = std::string(typeId);
		registration.classShortName = std::string(classShortName);
		registration.propertyBaseShortName = std::string(propertyBaseShortName);
		registration.createInstance = []() -> std::unique_ptr<IPropertiesProvider> {
			return std::make_unique<TMetaClass>();
		};
		registration.isType = [](const IPropertiesProvider* provider) {
			return dynamic_cast<const TMetaClass*>(provider) != nullptr;
		};
		MetaClassRegistrationDetail::AppendMetaClassRegistration(std::move(registration));
	}

} // namespace Engine::Serialization
