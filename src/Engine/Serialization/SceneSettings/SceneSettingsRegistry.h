#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <pugixml.hpp>

namespace Engine::Serialization {

	class SceneSettingsRegistry
	{
	public:
		static SceneSettingsRegistry& GetInstance();

		void SaveAll(pugi::xml_node settingsParent, SerializationResult& result) const;
		void LoadAll(const pugi::xml_node& settingsParent, SerializationResult& result) const;
		void ApplyAllDefaults() const;

	private:
		SceneSettingsRegistry();
	};

} // namespace Engine::Serialization
