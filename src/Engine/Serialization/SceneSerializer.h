#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <filesystem>
#include <memory>
#include <utility>

class Scene;

namespace Engine::Serialization {

	/// Legacy entry points; prefer SceneDocumentSerializer for Scene/SceneObject-aware IO.
	class SceneSerializer
	{
	public:
		static SerializationResult SaveSceneToFile(Scene& scene, const std::filesystem::path& path);
		static std::pair<std::shared_ptr<Scene>, SerializationResult> LoadSceneFromFile(
		    const std::filesystem::path& path);
	};

} // namespace Engine::Serialization
