#include "Engine/Serialization/SceneSerializer.h"

#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Serialization/SceneNodeTreeSerializer.h"

#include <pugixml.hpp>

#include <filesystem>
#include <memory>
#include <utility>

namespace Engine::Serialization {
	namespace {

		constexpr const char kSceneElement[] = "Scene";
		constexpr const char kNodeElement[] = "Node";
		constexpr const char kVersionAttr[] = "version";

	} // namespace

	SerializationResult SceneSerializer::SaveSceneToFile(Scene& scene, const std::filesystem::path& path) {
		SerializationResult result;

		scene.RebuildObjectIndex();

		const auto sceneRoot = scene.GetRoot();
		if (!sceneRoot) {
			result.AddError(kSceneElement, "Scene root is null");
			return result;
		}

		std::error_code filesystemError;
		const std::filesystem::path parentPath = path.parent_path();
		if (!parentPath.empty()) {
			std::filesystem::create_directories(parentPath, filesystemError);
		}
		if (filesystemError) {
			result.AddError(path.string(), "Failed to create directories for output file");
			return result;
		}

		pugi::xml_document document;
		pugi::xml_node sceneNode = document.append_child(kSceneElement);
		sceneNode.append_attribute(kVersionAttr).set_value(1);
		SceneNodeTreeSerializer::SaveNode(*sceneRoot, sceneNode, result);

		const bool saved = document.save_file(path.string().c_str(), "  ", pugi::format_default, pugi::encoding_utf8);
		if (!saved) {
			result.AddError(path.string(), "Failed to save XML file");
		}

		return result;
	}

	std::pair<std::shared_ptr<Scene>, SerializationResult> SceneSerializer::LoadSceneFromFile(
	    const std::filesystem::path& path) {
		SerializationResult result;
		auto scene = std::make_shared<Scene>();

		pugi::xml_document document;
		const pugi::xml_parse_result parseResult = document.load_file(path.string().c_str());
		if (!parseResult) {
			result.AddError(path.string(), "Failed to parse XML file");
			return {nullptr, result};
		}

		const pugi::xml_node sceneNode = document.child(kSceneElement);
		if (!sceneNode) {
			result.AddError(path.string(), "XML does not contain Scene root element");
			return {nullptr, result};
		}

		const pugi::xml_node rootNode = sceneNode.child(kNodeElement);
		if (!rootNode) {
			result.AddError(path.string(), "Scene XML does not contain root Node");
			return {nullptr, result};
		}

		if (auto root = scene->GetRoot()) {
			SceneNodeTreeSerializer::LoadNode(rootNode, *root, result);
		}
		if (!result.isSuccess) {
			return {nullptr, result};
		}
		scene->RebuildObjectIndex();
		return {std::move(scene), result};
	}

} // namespace Engine::Serialization
