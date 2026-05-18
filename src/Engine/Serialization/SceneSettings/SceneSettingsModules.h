#pragma once

#include "Engine/Serialization/SceneSettings/ISceneSettingsModule.h"

namespace Engine::Serialization {

	[[nodiscard]] const ISceneSettingsModule& GetSimulationSettingsModule();
	[[nodiscard]] const ISceneSettingsModule& GetPhysicsSettingsModule();
	[[nodiscard]] const ISceneSettingsModule& GetLightingSettingsModule();
	[[nodiscard]] const ISceneSettingsModule& GetBackgroundSettingsModule();

} // namespace Engine::Serialization
