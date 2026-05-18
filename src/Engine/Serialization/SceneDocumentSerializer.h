#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <filesystem>
#include <memory>

class Scene;
class SceneNode;

namespace Engine::Serialization {

	enum class SceneDocumentKind
	{
		Scene,
		Prefab
	};

	struct SceneDocumentLoadResult
	{
		std::shared_ptr<Scene> scene;
		SceneDocumentKind kind = SceneDocumentKind::Scene;
		SerializationResult result;
	};

	class SceneDocumentSerializer
	{
	public:
		static SerializationResult SaveSceneDocument(Scene& scene, const std::filesystem::path& path);
		static SerializationResult SavePrefabDocument(const SceneNode& prefabRoot, const std::filesystem::path& path);
		static SceneDocumentLoadResult LoadDocumentFromFile(const std::filesystem::path& path);
	};

} // namespace Engine::Serialization
