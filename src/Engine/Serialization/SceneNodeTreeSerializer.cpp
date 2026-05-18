#include "Engine/Serialization/SceneNodeTreeSerializer.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneObjectId.h"
#include "Engine/Core/Transform.h"
#include "Engine/Serialization/PropertyTreeSerializer.h"
#include "Engine/Serialization/SceneEntityRegistry.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <memory>
#include <string>

namespace Engine::Serialization {
	namespace {

		constexpr const char kNodeElement[] = "Node";
		constexpr const char kPropertiesElement[] = "Properties";
		constexpr const char kTransformElement[] = "Transform";
		constexpr const char kTransformTypeId[] = "Engine.Transform";
		constexpr const char kVisualElement[] = "Visual";
		constexpr const char kSortingElement[] = "SortingStrategy";
		constexpr const char kBehavioursElement[] = "Behaviours";
		constexpr const char kBehaviourElement[] = "Behaviour";
		constexpr const char kChildrenElement[] = "Children";
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

		void ClearNodeContent(SceneNode& sceneNode) {
			while (!sceneNode.GetChildren().empty()) {
				const auto& child = sceneNode.GetChildren().back();
				sceneNode.RemoveChild(child.get());
			}
			while (!sceneNode.GetBehaviours().empty()) {
				sceneNode.RemoveBehaviour(sceneNode.GetBehaviours().back().get());
			}
			std::shared_ptr<Visual> emptyVisual;
			sceneNode.SetVisual(std::move(emptyVisual));
			sceneNode.SetSortingStrategy(nullptr);
		}

		void LoadNodeRecursive(const pugi::xml_node& xmlNode, SceneNode& targetNode, SerializationResult& result) {
			if (!xmlNode) {
				result.AddError(kNodeElement, "Missing XML node");
				return;
			}

			if (const pugi::xml_attribute oidAttr = xmlNode.attribute(kObjectIdAttr)) {
				targetNode.SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
			}

			ClearNodeContent(targetNode);
			if (const pugi::xml_node propertiesNode = xmlNode.child(kPropertiesElement)) {
				result.Merge(PropertyTreeSerializer::LoadProvider(targetNode, propertiesNode));
			}

			if (const pugi::xml_node transformNode = xmlNode.child(kTransformElement)) {
				if (const pugi::xml_node transformProperties = transformNode.child(kPropertiesElement)) {
					result.Merge(
					    PropertyTreeSerializer::LoadProvider(targetNode.GetLocalTransform(), transformProperties));
					targetNode.MarkWorldTransformSubtreeDirty();
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
					targetNode.SetVisual(std::move(visual));
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
					targetNode.SetSortingStrategy(sorting);
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
					targetNode.AddBehaviour(std::move(behaviour));
				}
			}

			if (const pugi::xml_node childrenNode = xmlNode.child(kChildrenElement)) {
				for (const pugi::xml_node xmlChild : childrenNode.children(kNodeElement)) {
					auto childSceneNode = SceneNode::Create();
					if (const pugi::xml_attribute oidAttr = xmlChild.attribute(kObjectIdAttr)) {
						childSceneNode->SetSceneObjectId(static_cast<Engine::SceneObjectId>(oidAttr.as_uint()));
					}
					LoadNodeRecursive(xmlChild, *childSceneNode, result);
					targetNode.AddChild(childSceneNode);
				}
			}
		}

	} // namespace

	void SceneNodeTreeSerializer::SaveNode(
	    const SceneNode& sceneNode, pugi::xml_node xmlParent, SerializationResult& result) {
		SaveNodeRecursive(std::const_pointer_cast<SceneNode>(sceneNode.shared_from_this()), xmlParent, result);
	}

	void SceneNodeTreeSerializer::LoadNode(
	    const pugi::xml_node& xmlNode, SceneNode& targetNode, SerializationResult& result) {
		LoadNodeRecursive(xmlNode, targetNode, result);
	}

} // namespace Engine::Serialization
