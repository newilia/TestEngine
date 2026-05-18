#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <pugixml.hpp>

#include <string_view>

namespace Engine::Serialization {

	class ISceneSettingsModule
	{
	public:
		virtual ~ISceneSettingsModule() = default;

		[[nodiscard]] virtual std::string_view GetElementName() const = 0;
		virtual void Save(pugi::xml_node settingsParent, SerializationResult& result) const = 0;
		virtual void Load(const pugi::xml_node& settingsParent, SerializationResult& result) const = 0;
		virtual void ApplyDefaults() const = 0;
	};

} // namespace Engine::Serialization
