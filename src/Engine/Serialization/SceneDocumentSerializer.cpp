#include "Engine/Serialization/SceneDocumentSerializer.h"

#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Serialization/SceneNodeTreeSerializer.h"
#include "Engine/Serialization/SceneSettings/SceneSettingsRegistry.h"

#include <pugixml.hpp>

#include <filesystem>
#include <utility>

namespace Engine::Serialization {
	namespace {

		constexpr const char kSceneElement[] = "Scene";
		constexpr const char kSceneObjectElement[] = "SceneObject";
		constexpr const char kSettingsElement[] = "Settings";
		constexpr const char kNodeElement[] = "Node";
		constexpr const char kVersionAttr[] = "version";
		constexpr int kSceneVersion = 2;
		constexpr int kSceneObjectVersion = 1;

		SerializationResult EnsureParentDirectories(const std::filesystem::path& path) {
			SerializationResult result;
			std::error_code filesystemError;
			const std::filesystem::path parentPath = path.parent_path();
			if (!parentPath.empty()) {
				std::filesystem::create_directories(parentPath, filesystemError);
			}
			if (filesystemError) {
				result.AddError(path.string(), "Failed to create directories for output file");
			}
			return result;
		}

		bool SaveXmlDocument(
		    pugi::xml_document& document, const std::filesystem::path& path, SerializationResult& result) {
			const bool saved =
			    document.save_file(path.string().c_str(), "  ", pugi::format_default, pugi::encoding_utf8);
			if (!saved) {
				result.AddError(path.string(), "Failed to save XML file");
			}
			return saved;
		}

	} // namespace

	SerializationResult SceneDocumentSerializer::SaveSceneDocument(Scene& scene, const std::filesystem::path& path) {
		SerializationResult result = EnsureParentDirectories(path);
		if (!result.isSuccess) {
			return result;
		}

		scene.RebuildEntityIndex();
		const auto sceneRoot = scene.GetRoot();
		if (!sceneRoot) {
			result.AddError(kSceneElement, "Scene root is null");
			return result;
		}

		pugi::xml_document document;
		pugi::xml_node sceneNode = document.append_child(kSceneElement);
		sceneNode.append_attribute(kVersionAttr).set_value(kSceneVersion);

		pugi::xml_node settingsNode = sceneNode.append_child(kSettingsElement);
		SceneSettingsRegistry::GetInstance().SaveAll(settingsNode, result);

		SceneNodeTreeSerializer::SaveNode(*sceneRoot, sceneNode, result);
		SaveXmlDocument(document, path, result);
		return result;
	}

	SerializationResult SceneDocumentSerializer::SaveSceneObjectDocument(
	    const SceneNode& sceneObjectRoot, const std::filesystem::path& path) {
		SerializationResult result = EnsureParentDirectories(path);
		if (!result.isSuccess) {
			return result;
		}

		pugi::xml_document document;
		pugi::xml_node sceneObjectNode = document.append_child(kSceneObjectElement);
		sceneObjectNode.append_attribute(kVersionAttr).set_value(kSceneObjectVersion);
		SceneNodeTreeSerializer::SaveNode(sceneObjectRoot, sceneObjectNode, result);
		SaveXmlDocument(document, path, result);
		return result;
	}

	SceneDocumentLoadResult SceneDocumentSerializer::LoadDocumentFromFile(const std::filesystem::path& path) {
		SceneDocumentLoadResult loadResult;

		pugi::xml_document document;
		const pugi::xml_parse_result parseResult = document.load_file(path.string().c_str());
		if (!parseResult) {
			loadResult.result.AddError(path.string(), "Failed to parse XML file");
			return loadResult;
		}

		const pugi::xml_node sceneElement = document.child(kSceneElement);
		const pugi::xml_node sceneObjectElement = document.child(kSceneObjectElement);

		if (sceneElement) {
			loadResult.kind = SceneDocumentKind::Scene;
			if (const pugi::xml_attribute versionAttr = sceneElement.attribute(kVersionAttr)) {
				if (versionAttr.as_int() > kSceneVersion) {
					loadResult.result.AddWarning(kSceneElement, "Scene version is newer than supported");
				}
			}
		}
		else if (sceneObjectElement) {
			loadResult.kind = SceneDocumentKind::SceneObject;
			if (const pugi::xml_attribute versionAttr = sceneObjectElement.attribute(kVersionAttr)) {
				if (versionAttr.as_int() > kSceneObjectVersion) {
					loadResult.result.AddWarning(kSceneObjectElement, "SceneObject version is newer than supported");
				}
			}
		}
		else {
			loadResult.result.AddError(path.string(), "XML root must be Scene or SceneObject");
			return loadResult;
		}

		const pugi::xml_node documentRoot = sceneElement ? sceneElement : sceneObjectElement;
		const pugi::xml_node rootNode = documentRoot.child(kNodeElement);
		if (!rootNode) {
			loadResult.result.AddError(path.string(), "Document XML does not contain root Node");
			return loadResult;
		}

		loadResult.scene = std::make_shared<Scene>();
		if (auto root = loadResult.scene->GetRoot()) {
			SceneNodeTreeSerializer::LoadNode(rootNode, *root, loadResult.result);
		}

		if (!loadResult.result.isSuccess) {
			loadResult.scene.reset();
			return loadResult;
		}

		if (sceneElement) {
			SceneSettingsRegistry::GetInstance().ApplyAllDefaults();
			if (const pugi::xml_node settingsNode = sceneElement.child(kSettingsElement)) {
				SceneSettingsRegistry::GetInstance().LoadAll(settingsNode, loadResult.result);
			}
		}

		if (!loadResult.result.isSuccess) {
			loadResult.scene.reset();
			return loadResult;
		}

		loadResult.scene->RebuildEntityIndex();
		return loadResult;
	}

} // namespace Engine::Serialization
