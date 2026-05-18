#include "Engine/Serialization/SceneSettings/SceneSettingsRegistry.h"

#include "Engine/Serialization/SceneSettings/SceneSettingsModules.h"

#include <array>

namespace Engine::Serialization {

	SceneSettingsRegistry& SceneSettingsRegistry::GetInstance() {
		static SceneSettingsRegistry instance;
		return instance;
	}

	SceneSettingsRegistry::SceneSettingsRegistry() = default;

	void SceneSettingsRegistry::SaveAll(pugi::xml_node settingsParent, SerializationResult& result) const {
		const std::array<const ISceneSettingsModule*, 6> modules = {
		    &GetSimulationSettingsModule(),
		    &GetViewSettingsModule(),
		    &GetPhysicsSettingsModule(),
		    &GetLightingSettingsModule(),
		    &GetVisualizationSettingsModule(),
		    &GetBackgroundSettingsModule(),
		};
		for (const ISceneSettingsModule* module : modules) {
			module->Save(settingsParent, result);
		}
	}

	void SceneSettingsRegistry::LoadAll(const pugi::xml_node& settingsParent, SerializationResult& result) const {
		const std::array<const ISceneSettingsModule*, 6> modules = {
		    &GetSimulationSettingsModule(),
		    &GetViewSettingsModule(),
		    &GetPhysicsSettingsModule(),
		    &GetLightingSettingsModule(),
		    &GetVisualizationSettingsModule(),
		    &GetBackgroundSettingsModule(),
		};
		for (const ISceneSettingsModule* module : modules) {
			module->Load(settingsParent, result);
		}
	}

	void SceneSettingsRegistry::ApplyAllDefaults() const {
		const std::array<const ISceneSettingsModule*, 6> modules = {
		    &GetSimulationSettingsModule(),
		    &GetViewSettingsModule(),
		    &GetPhysicsSettingsModule(),
		    &GetLightingSettingsModule(),
		    &GetVisualizationSettingsModule(),
		    &GetBackgroundSettingsModule(),
		};
		for (const ISceneSettingsModule* module : modules) {
			module->ApplyDefaults();
		}
	}

} // namespace Engine::Serialization
