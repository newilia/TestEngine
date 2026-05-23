#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <filesystem>
#include <memory>

class Scene;
class SceneNode;

namespace Engine::Serialization {

	/// Kind of on-disk scene XML document (full scene vs reusable subtree file).
	enum class SceneDocumentKind
	{
		Scene,
		SceneObject
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
		static SerializationResult SaveSceneObjectDocument(
		    const SceneNode& sceneObjectRoot, const std::filesystem::path& path);
		static SceneDocumentLoadResult LoadDocumentFromFile(const std::filesystem::path& path);
	};

} // namespace Engine::Serialization
