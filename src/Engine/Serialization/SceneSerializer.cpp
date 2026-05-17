#include "Engine/Serialization/SceneSerializer.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneObjectId.h"
#include "Engine/Core/Transform.h"
#include "Engine/Serialization/PropertyTreeSerializer.h"
#include "Engine/Serialization/SceneEntityRegistry.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <pugixml.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace Engine::Serialization {
	namespace {

		constexpr const char kSceneElement[] = "Scene";
		constexpr const char kNodeElement[] = "Node";
		constexpr const char kPropertiesElement[] = "Properties";
		constexpr const char kTransformElement[] = "Transform";
		constexpr const char kTransformTypeId[] = "Engine.Transform";
		constexpr const char kVisualElement[] = "Visual";
		constexpr const char kSortingElement[] = "SortingStrategy";
		constexpr const char kBehavioursElement[] = "Behaviours";
		constexpr const char kBehaviourElement[] = "Behaviour";
		constexpr const char kChildrenElement[] = "Children";
		constexpr const char kVersionAttr[] = "version";
		constexpr const char kTypeAttr[] = "type";
		constexpr const char kObjectIdAttr[] = "objectId";

		void WriteObjectIdAttr(pugi::xml_node xmlElement, Engine::SceneObjectId id) {
			xmlElement.append_attribute(kObjectIdAttr).set_value(id);
		}

		void SaveNodeRecursive(
		    const std::shared_ptr<SceneNode>& sceneNode, pugi::xml_node xmlParent, SerializationResult& result) {
			if (!sceneNode) {
				result.AddError(kNodeElement, "Scene node is null");
				return;
			}

			pugi::xml_node xmlNode = xmlParent.append_child(kNodeElement);
			WriteObjectIdAttr(xmlNode, sceneNode->GetSceneObjectId());
			pugi::xml_node nodeProperties = xmlNode.append_child(kPropertiesElement);
			result.Merge(PropertyTreeSerializer::SaveProvider(*sceneNode, nodeProperties));

			const Transform& transform = sceneNode->GetLocalTransform();
			pugi::xml_node transformNode = xmlNode.append_child(kTransformElement);
			transformNode.append_attribute(kTypeAttr).set_value(kTransformTypeId);
			pugi::xml_node transformProperties = transformNode.append_child(kPropertiesElement);
			result.Merge(PropertyTreeSerializer::SaveProvider(sceneNode->GetLocalTransform(), transformProperties));

			if (const auto visual = sceneNode->GetVisual()) {
				const std::string visualTypeId = SceneEntityRegistry::GetInstance().GetTypeIdForEntity(visual);
				if (visualTypeId.empty()) {
					result.AddWarning("Visual", "Visual type is not registered and will be skipped");
				}
				else {
					pugi::xml_node visualNode = xmlNode.append_child(kVisualElement);
					visualNode.append_attribute(kTypeAttr).set_value(visualTypeId.c_str());
					WriteObjectIdAttr(visualNode, visual->GetSceneObjectId());
					pugi::xml_node visualProperties = visualNode.append_child(kPropertiesElement);
					result.Merge(PropertyTreeSerializer::SaveProvider(*visual, visualProperties));
				}
			}

			if (const auto sorting = sceneNode->GetSortingStrategy()) {
				const std::string sortingTypeId = SceneEntityRegistry::GetInstance().GetTypeIdForEntity(sorting);
				if (sortingTypeId.empty()) {
					result.AddWarning("SortingStrategy", "Sorting strategy type is not registered and will be skipped");
				}
				else {
					pugi::xml_node sortingNode = xmlNode.append_child(kSortingElement);
					sortingNode.append_attribute(kTypeAttr).set_value(sortingTypeId.c_str());
					WriteObjectIdAttr(sortingNode, sorting->GetSceneObjectId());
					pugi::xml_node sortingProperties = sortingNode.append_child(kPropertiesElement);
					result.Merge(PropertyTreeSerializer::SaveProvider(*sorting, sortingProperties));
				}
			}

			pugi::xml_node behavioursNode = xmlNode.append_child(kBehavioursElement);
			for (const auto& behaviour : sceneNode->GetBehaviours()) {
				const std::string behaviourTypeId = SceneEntityRegistry::GetInstance().GetTypeIdForEntity(behaviour);
				if (behaviourTypeId.empty()) {
					result.AddWarning("Behaviour", "Behaviour type is not registered and will be skipped");
					continue;
				}
				pugi::xml_node behaviourNode = behavioursNode.append_child(kBehaviourElement);
				behaviourNode.append_attribute(kTypeAttr).set_value(behaviourTypeId.c_str());
				WriteObjectIdAttr(behaviourNode, behaviour->GetSceneObjectId());
				pugi::xml_node behaviourProperties = behaviourNode.append_child(kPropertiesElement);
				result.Merge(PropertyTreeSerializer::SaveProvider(*behaviour, behaviourProperties));
			}

			pugi::xml_node childrenNode = xmlNode.append_child(kChildrenElement);
			for (const auto& child : sceneNode->GetChildren()) {
				SaveNodeRecursive(child, childrenNode, result);
			}
		}

		void ClearNodeContent(const std::shared_ptr<SceneNode>& sceneNode) {
			if (!sceneNode) {
				return;
			}
			while (!sceneNode->GetChildren().empty()) {
				const auto& child = sceneNode->GetChildren().back();
				sceneNode->RemoveChild(child.get());
			}
			while (!sceneNode->GetBehaviours().empty()) {
				sceneNode->RemoveBehaviour(sceneNode->GetBehaviours().back().get());
			}
			std::shared_ptr<Visual> emptyVisual;
			sceneNode->SetVisual(std::move(emptyVisual));
			sceneNode->SetSortingStrategy(nullptr);
		}

		void LoadNodeRecursive(
		    const pugi::xml_node& xmlNode, const std::shared_ptr<SceneNode>& targetNode, SerializationResult& result) {
			if (!xmlNode || !targetNode) {
				result.AddError(kNodeElement, "Missing XML node or target scene node");
				return;
			}

			if (const pugi::xml_attribute oidAttr = xmlNode.attribute(kObjectIdAttr)) {
				targetNode->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
			}

			ClearNodeContent(targetNode);
			if (const pugi::xml_node propertiesNode = xmlNode.child(kPropertiesElement)) {
				result.Merge(PropertyTreeSerializer::LoadProvider(*targetNode, propertiesNode));
			}

			if (const pugi::xml_node transformNode = xmlNode.child(kTransformElement)) {
				if (const pugi::xml_node transformProperties = transformNode.child(kPropertiesElement)) {
					result.Merge(
					    PropertyTreeSerializer::LoadProvider(targetNode->GetLocalTransform(), transformProperties));
					targetNode->MarkWorldTransformSubtreeDirty();
				}
			}

			if (const pugi::xml_node visualNode = xmlNode.child(kVisualElement)) {
				const std::string visualTypeId = visualNode.attribute(kTypeAttr).as_string();
				const std::shared_ptr<EntityOnNode> created =
				    SceneEntityRegistry::GetInstance().CreateByTypeId(visualTypeId);
				if (!created) {
					result.AddError(kVisualElement, "Unknown visual type id: " + visualTypeId);
				}
				else if (auto visual = std::dynamic_pointer_cast<Visual>(created)) {
					if (const pugi::xml_attribute oidAttr = visualNode.attribute(kObjectIdAttr)) {
						visual->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
					}
					if (const pugi::xml_node visualProperties = visualNode.child(kPropertiesElement)) {
						result.Merge(PropertyTreeSerializer::LoadProvider(*visual, visualProperties));
					}
					targetNode->SetVisual(std::move(visual));
				}
				else {
					result.AddError(kVisualElement, "Registered type is not a Visual: " + visualTypeId);
				}
			}

			if (const pugi::xml_node sortingNode = xmlNode.child(kSortingElement)) {
				const std::string sortingTypeId = sortingNode.attribute(kTypeAttr).as_string();
				const std::shared_ptr<EntityOnNode> created =
				    SceneEntityRegistry::GetInstance().CreateByTypeId(sortingTypeId);
				if (!created) {
					result.AddError(kSortingElement, "Unknown sorting strategy type id: " + sortingTypeId);
				}
				else if (auto sorting = std::dynamic_pointer_cast<RelativeSortingStrategy>(created)) {
					if (const pugi::xml_attribute oidAttr = sortingNode.attribute(kObjectIdAttr)) {
						sorting->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
					}
					if (const pugi::xml_node sortingProperties = sortingNode.child(kPropertiesElement)) {
						result.Merge(PropertyTreeSerializer::LoadProvider(*sorting, sortingProperties));
					}
					targetNode->SetSortingStrategy(sorting);
				}
				else {
					result.AddError(kSortingElement, "Registered type is not a sorting strategy: " + sortingTypeId);
				}
			}

			if (const pugi::xml_node behavioursNode = xmlNode.child(kBehavioursElement)) {
				for (const pugi::xml_node behaviourNode : behavioursNode.children(kBehaviourElement)) {
					const std::string behaviourTypeId = behaviourNode.attribute(kTypeAttr).as_string();
					const std::shared_ptr<EntityOnNode> created =
					    SceneEntityRegistry::GetInstance().CreateByTypeId(behaviourTypeId);
					if (!created) {
						result.AddError(kBehaviourElement, "Unknown behaviour type id: " + behaviourTypeId);
						continue;
					}
					auto behaviour = std::dynamic_pointer_cast<Behaviour>(created);
					if (!behaviour) {
						result.AddError(kBehaviourElement, "Registered type is not a Behaviour: " + behaviourTypeId);
						continue;
					}
					if (const pugi::xml_attribute oidAttr = behaviourNode.attribute(kObjectIdAttr)) {
						behaviour->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
					}
					if (const pugi::xml_node behaviourProperties = behaviourNode.child(kPropertiesElement)) {
						result.Merge(PropertyTreeSerializer::LoadProvider(*behaviour, behaviourProperties));
					}
					targetNode->AddBehaviour(std::move(behaviour));
				}
			}

			if (const pugi::xml_node childrenNode = xmlNode.child(kChildrenElement)) {
				for (const pugi::xml_node xmlChild : childrenNode.children(kNodeElement)) {
					auto childSceneNode = SceneNode::Create();
					if (const pugi::xml_attribute oidAttr = xmlChild.attribute(kObjectIdAttr)) {
						childSceneNode->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
					}
					LoadNodeRecursive(xmlChild, childSceneNode, result);
					targetNode->AddChild(childSceneNode);
				}
			}
		}

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
		SaveNodeRecursive(sceneRoot, sceneNode, result);

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

		LoadNodeRecursive(rootNode, scene->GetRoot(), result);
		if (!result.isSuccess) {
			return {nullptr, result};
		}
		scene->RebuildObjectIndex();
		return {std::move(scene), result};
	}

} // namespace Engine::Serialization
