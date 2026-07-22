#include "Engine/Core/SceneNodeClone.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EntityIdUtils.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/PropertyNode.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Serialization/SceneEntityRegistry.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Engine {
	namespace {

		using EntityIdMap = std::unordered_map<EntityId, EntityId>;

		void AlignContainerSizes(const PropertyNode& source, PropertyNode& target) {
			if (source.kind != target.kind || source.id != target.id) {
				return;
			}
			if (source.kind == PropertyKind::Sequence) {
				if (const auto* sourceOps = std::get_if<PropAccessSequence>(&source.access)) {
					if (const auto* targetOps = std::get_if<PropAccessSequence>(&target.access)) {
						if (sourceOps->getSize && targetOps->resize) {
							targetOps->resize(sourceOps->getSize());
						}
					}
				}
				return;
			}
			if (source.kind == PropertyKind::Associative) {
				if (const auto* sourceOps = std::get_if<PropAccessAssociative>(&source.access)) {
					if (const auto* targetOps = std::get_if<PropAccessAssociative>(&target.access)) {
						std::size_t sourceSize = source.children.size();
						std::size_t targetSize = target.children.size();
						while (targetSize < sourceSize && targetOps->addPair) {
							targetOps->addPair();
							++targetSize;
						}
						while (targetSize > sourceSize && targetOps->removePair) {
							targetOps->removePair(targetSize - 1);
							--targetSize;
						}
					}
				}
				return;
			}
			const std::size_t count = std::min(source.children.size(), target.children.size());
			for (std::size_t i = 0; i < count; ++i) {
				AlignContainerSizes(source.children[i], target.children[i]);
			}
		}

		void ClearSceneRefsInPropertyNode(PropertyNode& node) {
			if (node.kind == PropertyKind::SceneRef) {
				if (auto* acc = std::get_if<PropAccessSceneRef>(&node.access)) {
					if (acc->set) {
						acc->set(0);
					}
				}
			}
			for (PropertyNode& child : node.children) {
				ClearSceneRefsInPropertyNode(child);
			}
		}

		void ClearSceneRefsInProvider(IPropertiesProvider& provider) {
			PropertyTree tree;
			PropertyBuilder builder(tree);
			provider.BuildPropertyTree(builder);
			for (PropertyNode& root : tree.roots) {
				ClearSceneRefsInPropertyNode(root);
			}
		}

		void RemapSceneRefsInPropertyNode(
		    PropertyNode& node, const EntityIdMap& remap, const std::unordered_set<EntityId>& sourceSubtreeIds) {
			if (node.kind == PropertyKind::SceneRef) {
				if (auto* acc = std::get_if<PropAccessSceneRef>(&node.access)) {
					if (acc->get && acc->set) {
						const EntityId refId = acc->get();
						if (refId != kInvalidEntityId && sourceSubtreeIds.contains(refId)) {
							if (const auto it = remap.find(refId); it != remap.end()) {
								acc->set(it->second);
							}
							else {
								acc->set(0);
							}
						}
						else {
							acc->set(0);
						}
					}
				}
			}
			for (PropertyNode& child : node.children) {
				RemapSceneRefsInPropertyNode(child, remap, sourceSubtreeIds);
			}
		}

		void RemapSceneRefsInProvider(IPropertiesProvider& provider, const EntityIdMap& remap,
		    const std::unordered_set<EntityId>& sourceSubtreeIds) {
			PropertyTree tree;
			PropertyBuilder builder(tree);
			provider.BuildPropertyTree(builder);
			for (PropertyNode& root : tree.roots) {
				RemapSceneRefsInPropertyNode(root, remap, sourceSubtreeIds);
			}
		}

		void RemapSceneRefsOnClonedSubtree(const std::shared_ptr<SceneNode>& root, const EntityIdMap& remap,
		    const std::unordered_set<EntityId>& sourceSubtreeIds) {
			if (!root) {
				return;
			}
			RemapSceneRefsInProvider(*root, remap, sourceSubtreeIds);
			RemapSceneRefsInProvider(root->GetLocalTransform(), remap, sourceSubtreeIds);
			if (const auto sorting = root->GetSortingStrategy()) {
				RemapSceneRefsInProvider(*sorting, remap, sourceSubtreeIds);
			}
			if (const auto visual = root->GetVisual()) {
				RemapSceneRefsInProvider(*visual, remap, sourceSubtreeIds);
			}
			for (const auto& behaviour : root->GetBehaviours()) {
				if (behaviour) {
					RemapSceneRefsInProvider(*behaviour, remap, sourceSubtreeIds);
				}
			}
			for (const auto& child : root->GetChildren()) {
				RemapSceneRefsOnClonedSubtree(child, remap, sourceSubtreeIds);
			}
		}

		void CollectEntityId(EntityId id, std::unordered_set<EntityId>& out) {
			if (id != kInvalidEntityId) {
				out.insert(id);
			}
		}

		void CollectEntityIdsInSubtree(const std::shared_ptr<SceneNode>& root, std::unordered_set<EntityId>& out) {
			if (!root) {
				return;
			}
			CollectEntityId(root->GetEntityId(), out);
			if (const auto visual = root->GetVisual()) {
				CollectEntityId(visual->GetEntityId(), out);
			}
			if (const auto sorting = root->GetSortingStrategy()) {
				CollectEntityId(sorting->GetEntityId(), out);
			}
			for (const auto& behaviour : root->GetBehaviours()) {
				if (behaviour) {
					CollectEntityId(behaviour->GetEntityId(), out);
				}
			}
			for (const auto& child : root->GetChildren()) {
				CollectEntityIdsInSubtree(child, out);
			}
		}

		void AssignFreshEntityId(EntityId sourceId, const std::function<void(EntityId)>& setCloneId, EntityIdMap& remap,
		    std::unordered_set<EntityId>& claimed) {
			EntityId newId = kInvalidEntityId;
			EnsureUniqueEntityId(newId, claimed);
			if (sourceId != kInvalidEntityId) {
				remap[sourceId] = newId;
			}
			setCloneId(newId);
		}

		void AssignFreshEntityIdsAndBuildRemap(const std::shared_ptr<SceneNode>& source,
		    const std::shared_ptr<SceneNode>& clone, std::unordered_set<EntityId>& claimed, EntityIdMap& remap) {
			if (!source || !clone) {
				return;
			}

			AssignFreshEntityId(
			    source->GetEntityId(),
			    [&](EntityId id) {
				    clone->SetEntityId(id);
			    },
			    remap, claimed);

			if (const auto sourceVisual = source->GetVisual()) {
				if (const auto cloneVisual = clone->GetVisual()) {
					AssignFreshEntityId(
					    sourceVisual->GetEntityId(),
					    [&](EntityId id) {
						    cloneVisual->SetEntityId(id);
					    },
					    remap, claimed);
				}
			}
			if (const auto sourceSorting = source->GetSortingStrategy()) {
				if (const auto cloneSorting = clone->GetSortingStrategy()) {
					AssignFreshEntityId(
					    sourceSorting->GetEntityId(),
					    [&](EntityId id) {
						    cloneSorting->SetEntityId(id);
					    },
					    remap, claimed);
				}
			}
			const auto& sourceBehaviours = source->GetBehaviours();
			const auto& cloneBehaviours = clone->GetBehaviours();
			const std::size_t behaviourCount = std::min(sourceBehaviours.size(), cloneBehaviours.size());
			for (std::size_t i = 0; i < behaviourCount; ++i) {
				if (sourceBehaviours[i] && cloneBehaviours[i]) {
					AssignFreshEntityId(
					    sourceBehaviours[i]->GetEntityId(),
					    [&](EntityId id) {
						    cloneBehaviours[i]->SetEntityId(id);
					    },
					    remap, claimed);
				}
			}

			const auto& sourceChildren = source->GetChildren();
			const auto& cloneChildren = clone->GetChildren();
			const std::size_t childCount = std::min(sourceChildren.size(), cloneChildren.size());
			for (std::size_t i = 0; i < childCount; ++i) {
				AssignFreshEntityIdsAndBuildRemap(sourceChildren[i], cloneChildren[i], claimed, remap);
			}
		}

		void CopyLeafValue(const PropertyNode& source, PropertyNode& target) {
			if (source.kind != target.kind || source.id != target.id) {
				return;
			}
			switch (source.kind) {
			case PropertyKind::Bool: {
				const auto* s = std::get_if<PropAccessBool>(&source.access);
				const auto* t = std::get_if<PropAccessBool>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Int32: {
				const auto* s = std::get_if<PropAccessInt32>(&source.access);
				const auto* t = std::get_if<PropAccessInt32>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Int64: {
				const auto* s = std::get_if<PropAccessInt64>(&source.access);
				const auto* t = std::get_if<PropAccessInt64>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Float: {
				const auto* s = std::get_if<PropAccessFloat>(&source.access);
				const auto* t = std::get_if<PropAccessFloat>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Double: {
				const auto* s = std::get_if<PropAccessDouble>(&source.access);
				const auto* t = std::get_if<PropAccessDouble>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::String: {
				const auto* s = std::get_if<PropAccessString>(&source.access);
				const auto* t = std::get_if<PropAccessString>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Enum: {
				const auto* s = std::get_if<PropAccessEnum>(&source.access);
				const auto* t = std::get_if<PropAccessEnum>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Vec2f: {
				const auto* s = std::get_if<PropAccessVec2f>(&source.access);
				const auto* t = std::get_if<PropAccessVec2f>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Vec2i: {
				const auto* s = std::get_if<PropAccessVec2i>(&source.access);
				const auto* t = std::get_if<PropAccessVec2i>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Vec2u: {
				const auto* s = std::get_if<PropAccessVec2u>(&source.access);
				const auto* t = std::get_if<PropAccessVec2u>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Vec3f: {
				const auto* s = std::get_if<PropAccessVec3f>(&source.access);
				const auto* t = std::get_if<PropAccessVec3f>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Color: {
				const auto* s = std::get_if<PropAccessColor>(&source.access);
				const auto* t = std::get_if<PropAccessColor>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::SceneRef: {
				const auto* s = std::get_if<PropAccessSceneRef>(&source.access);
				const auto* t = std::get_if<PropAccessSceneRef>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::AssetRef: {
				const auto* s = std::get_if<PropAccessAssetRef>(&source.access);
				const auto* t = std::get_if<PropAccessAssetRef>(&target.access);
				if (s && t && s->get && t->set) {
					t->set(s->get());
				}
				return;
			}
			case PropertyKind::Object:
			case PropertyKind::Sequence:
			case PropertyKind::Associative:
				break;
			}
		}

		void CopyLeafValuesRecursive(const PropertyNode& source, PropertyNode& target) {
			if (source.kind != target.kind || source.id != target.id) {
				return;
			}
			CopyLeafValue(source, target);
			const std::size_t count = std::min(source.children.size(), target.children.size());
			for (std::size_t i = 0; i < count; ++i) {
				CopyLeafValuesRecursive(source.children[i], target.children[i]);
			}
		}

		std::shared_ptr<EntityOnNode> CloneEntityImpl(
		    const std::shared_ptr<EntityOnNode>& source, bool clearSceneRefs) {
			if (!source) {
				return nullptr;
			}
			const auto& registry = Serialization::SceneEntityRegistry::GetInstance();
			const std::string typeId = registry.GetTypeIdForEntity(source);
			if (typeId.empty()) {
				return nullptr;
			}
			auto target = registry.CreateByTypeId(typeId);
			if (!target) {
				return nullptr;
			}
			if (!CopyReflectedProperties(*source, *target)) {
				return nullptr;
			}
			if (clearSceneRefs) {
				ClearSceneRefsInProvider(*target);
			}
			return target;
		}

		std::shared_ptr<SceneNode> CloneSceneNodeRecursive(const std::shared_ptr<SceneNode>& source) {
			if (!source) {
				return nullptr;
			}
			auto clone = SceneNode::Create();
			clone->SetName(source->GetName());
			clone->SetEnabled(source->IsEnabled());
			clone->SetVisible(source->IsVisible());
			clone->CopyLocalTransformFrom(source->GetLocalTransform());

			if (auto visualClone = std::dynamic_pointer_cast<Visual>(CloneEntityImpl(source->GetVisual(), false))) {
				clone->SetVisual(std::move(visualClone));
			}
			if (auto sortingClone =
			        std::dynamic_pointer_cast<SortingStrategy>(CloneEntityImpl(source->GetSortingStrategy(), false))) {
				clone->SetSortingStrategy(sortingClone);
			}
			for (const auto& behaviour : source->GetBehaviours()) {
				if (auto behaviourClone = std::dynamic_pointer_cast<Behaviour>(CloneEntityImpl(behaviour, false))) {
					clone->AddBehaviour(std::move(behaviourClone));
				}
			}
			for (const auto& child : source->GetChildren()) {
				if (auto childClone = CloneSceneNodeRecursive(child)) {
					clone->AddChild(childClone);
				}
			}
			return clone;
		}

		void RemapSceneRefsForClonedSubtree(
		    const std::shared_ptr<SceneNode>& source, const std::shared_ptr<SceneNode>& clone) {
			std::unordered_set<EntityId> sourceSubtreeIds;
			CollectEntityIdsInSubtree(source, sourceSubtreeIds);

			std::unordered_set<EntityId> reservedIds;
			if (const auto scene = MainContext::GetInstance().GetScene()) {
				CollectEntityIdsInSubtree(scene->GetRoot(), reservedIds);
			}

			EntityIdMap remap;
			AssignFreshEntityIdsAndBuildRemap(source, clone, reservedIds, remap);
			RemapSceneRefsOnClonedSubtree(clone, remap, sourceSubtreeIds);
		}

	} // namespace

	bool CopyReflectedProperties(IPropertiesProvider& source, IPropertiesProvider& target) {
		PropertyTree sourceTree;
		PropertyTree targetTree;
		PropertyBuilder sourceBuilder(sourceTree);
		PropertyBuilder targetBuilder(targetTree);
		source.BuildPropertyTree(sourceBuilder);
		target.BuildPropertyTree(targetBuilder);

		if (sourceTree.roots.size() != targetTree.roots.size()) {
			return false;
		}
		for (std::size_t i = 0; i < sourceTree.roots.size(); ++i) {
			AlignContainerSizes(sourceTree.roots[i], targetTree.roots[i]);
		}

		sourceTree.roots.clear();
		targetTree.roots.clear();
		sourceBuilder.clear();
		targetBuilder.clear();
		source.BuildPropertyTree(sourceBuilder);
		target.BuildPropertyTree(targetBuilder);
		if (sourceTree.roots.size() != targetTree.roots.size()) {
			return false;
		}
		for (std::size_t i = 0; i < sourceTree.roots.size(); ++i) {
			CopyLeafValuesRecursive(sourceTree.roots[i], targetTree.roots[i]);
		}
		return true;
	}

	std::shared_ptr<Transform> CloneTransform(const Transform& source) {
		auto target = std::make_shared<Transform>();
		if (CopyReflectedProperties(const_cast<Transform&>(source), *target)) {
			ClearSceneRefsInProvider(*target);
			return target;
		}
		return nullptr;
	}

	std::shared_ptr<EntityOnNode> CloneEntity(const std::shared_ptr<EntityOnNode>& source) {
		return CloneEntityImpl(source, true);
	}

	std::shared_ptr<SceneNode> CloneSceneNode(const std::shared_ptr<SceneNode>& source) {
		const auto clone = CloneSceneNodeRecursive(source);
		if (!clone) {
			return nullptr;
		}
		RemapSceneRefsForClonedSubtree(source, clone);
		return clone;
	}

	std::shared_ptr<Scene> CloneScene(const std::shared_ptr<Scene>& source) {
		if (!source) {
			return nullptr;
		}
		auto clone = std::make_shared<Scene>();
		clone->SetRoot(CloneSceneNode(source->GetRoot()));
		return clone;
	}

} // namespace Engine
