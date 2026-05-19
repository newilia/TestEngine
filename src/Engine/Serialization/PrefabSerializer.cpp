#include "Engine/Serialization/PrefabSerializer.h"

#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeClone.h"
#include "Engine/Serialization/SceneDocumentSerializer.h"

namespace Engine::Serialization {

	PrefabInstantiateResult PrefabSerializer::InstantiateFromFile(const std::filesystem::path& path) {
		PrefabInstantiateResult instantiateResult;

		const SceneDocumentLoadResult loaded = SceneDocumentSerializer::LoadDocumentFromFile(path);
		instantiateResult.result = loaded.result;
		if (!loaded.result.isSuccess) {
			return instantiateResult;
		}
		if (loaded.kind != SceneDocumentKind::Prefab) {
			instantiateResult.result.AddError(path.string(), "File is not a Prefab document");
			return instantiateResult;
		}
		if (!loaded.scene) {
			instantiateResult.result.AddError(path.string(), "Prefab document did not produce a scene");
			return instantiateResult;
		}
		const std::shared_ptr<SceneNode> root = loaded.scene->GetRoot();
		if (!root) {
			instantiateResult.result.AddError(path.string(), "Prefab document root node is missing");
			return instantiateResult;
		}
		instantiateResult.instance = CloneSceneNode(root);
		if (!instantiateResult.instance) {
			instantiateResult.result.AddError(path.string(), "Failed to clone prefab root node");
		}
		return instantiateResult;
	}

} // namespace Engine::Serialization
