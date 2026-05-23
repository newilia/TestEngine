#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <filesystem>
#include <memory>

class SceneNode;

namespace Engine::Serialization {

	struct SceneObjectInstantiateResult
	{
		std::shared_ptr<SceneNode> instance;
		SerializationResult result;
	};

	class SceneObjectSerializer
	{
	public:
		static SceneObjectInstantiateResult InstantiateFromFile(const std::filesystem::path& path);
	};

} // namespace Engine::Serialization
