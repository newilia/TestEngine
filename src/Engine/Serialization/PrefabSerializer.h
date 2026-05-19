#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <filesystem>
#include <memory>

class SceneNode;

namespace Engine::Serialization {

	struct PrefabInstantiateResult
	{
		std::shared_ptr<SceneNode> instance;
		SerializationResult result;
	};

	class PrefabSerializer
	{
	public:
		static PrefabInstantiateResult InstantiateFromFile(const std::filesystem::path& path);
	};

} // namespace Engine::Serialization
