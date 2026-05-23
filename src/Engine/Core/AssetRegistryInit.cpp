#include "Engine/Core/AssetRegistrar.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneObject.h"
#include "Engine/Serialization/SceneObjectSerializer.h"
#include "Engine/Serialization/SceneSerializer.h"

namespace {

	std::shared_ptr<Scene> LoadSceneAsset(const std::filesystem::path& absolutePath) {
		const auto [scene, result] = Engine::Serialization::SceneSerializer::LoadSceneFromFile(absolutePath);
		(void)result;
		return scene;
	}

	std::shared_ptr<SceneObject> LoadSceneObjectAsset(const std::filesystem::path& absolutePath) {
		const Engine::Serialization::SceneObjectInstantiateResult result =
		    Engine::Serialization::SceneObjectSerializer::InstantiateFromFile(absolutePath);
		if (!result.instance) {
			return nullptr;
		}
		return std::make_shared<SceneObject>(result.instance);
	}

	const bool kRegisterSceneAsset = []() {
		Engine::RegisterAsset<Scene>("Engine.Scene", &LoadSceneAsset);
		return true;
	}();

	const bool kRegisterSceneObjectAsset = []() {
		Engine::RegisterAsset<SceneObject>("Engine.SceneObject", &LoadSceneObjectAsset);
		return true;
	}();

} // namespace
