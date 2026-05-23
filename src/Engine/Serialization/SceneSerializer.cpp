#include "Engine/Serialization/SceneSerializer.h"

#include "Engine/Core/Scene.h"
#include "Engine/Serialization/SceneDocumentSerializer.h"

namespace Engine::Serialization {

	SerializationResult SceneSerializer::SaveSceneToFile(Scene& scene, const std::filesystem::path& path) {
		return SceneDocumentSerializer::SaveSceneDocument(scene, path);
	}

	std::pair<std::shared_ptr<Scene>, SerializationResult> SceneSerializer::LoadSceneFromFile(
	    const std::filesystem::path& path) {
		SceneDocumentLoadResult loaded = SceneDocumentSerializer::LoadDocumentFromFile(path);
		if (!loaded.result.isSuccess || !loaded.scene) {
			return {nullptr, loaded.result};
		}
		if (loaded.kind != SceneDocumentKind::Scene) {
			loaded.result.AddError(path.string(), "File is a SceneObject document, not a Scene");
			return {nullptr, loaded.result};
		}
		return {std::move(loaded.scene), loaded.result};
	}

} // namespace Engine::Serialization
