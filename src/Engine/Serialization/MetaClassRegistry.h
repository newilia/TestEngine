#pragma once

#include "Engine/Core/IPropertiesProvider.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Engine::Serialization {

	struct MetaClassRegistration
	{
		std::string typeId;
		std::string classShortName;
		std::string propertyBaseShortName;
		std::function<std::unique_ptr<IPropertiesProvider>()> createInstance;
		std::function<bool(const IPropertiesProvider*)> isType;
	};

	class MetaClassRegistry
	{
	public:
		static const MetaClassRegistry& GetInstance();

		const std::vector<MetaClassRegistration>& GetAll() const;

		std::unique_ptr<IPropertiesProvider> CreateUniqueByTypeId(std::string_view typeId) const;

		template <typename TMetaClassBase>
		std::unique_ptr<TMetaClassBase> CreateUniqueByTypeIdAs(std::string_view typeId) const {
			std::unique_ptr<IPropertiesProvider> created = CreateUniqueByTypeId(typeId);
			if (!created) {
				return nullptr;
			}
			TMetaClassBase* derived = dynamic_cast<TMetaClassBase*>(created.get());
			if (!derived) {
				return nullptr;
			}
			created.release();
			return std::unique_ptr<TMetaClassBase>(derived);
		}

		std::string GetTypeIdForInstance(const IPropertiesProvider& instance) const;
		std::vector<std::string> GetDerivedTypeIds(std::string_view baseClassShortName) const;

	private:
		MetaClassRegistry();

		std::vector<MetaClassRegistration> _registrations;
	};

} // namespace Engine::Serialization
