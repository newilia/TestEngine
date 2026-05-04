#include "Engine/Editor/SceneCloneUtils.h"

#include "Engine/Behaviour/ButtonBehaviour.h"
#include "Engine/Behaviour/FpsCounterBehaviour.h"
#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/PropertyNode.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/CircleShapeVisual.h"
#include "Engine/Visual/ConvexShapeVisual.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "Engine/Visual/VectorArrowVisual.h"
#include "Environments/Demo1/SolarSystemBehaviour.h"
#include "Environments/Pong/AiPlatformControllerBehaviour.h"
#include "Environments/Pong/UserPlatformControllerBehaviour.h"

#include <algorithm>

namespace Engine {
	namespace {

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

		template <typename T>
		std::shared_ptr<EntityOnNode> MakeAndCopy(const std::shared_ptr<EntityOnNode>& source) {
			auto target = std::make_shared<T>();
			if (CopyReflectedProperties(*source, *target)) {
				return target;
			}
			return nullptr;
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

	std::shared_ptr<EntityOnNode> CloneEntity(const std::shared_ptr<EntityOnNode>& source) {
		if (!source) {
			return nullptr;
		}
		if (std::dynamic_pointer_cast<Transform>(source)) {
			return MakeAndCopy<Transform>(source);
		}
		if (std::dynamic_pointer_cast<CircleShapeVisual>(source)) {
			return MakeAndCopy<CircleShapeVisual>(source);
		}
		if (std::dynamic_pointer_cast<RectangleShapeVisual>(source)) {
			return MakeAndCopy<RectangleShapeVisual>(source);
		}
		if (std::dynamic_pointer_cast<ConvexShapeVisual>(source)) {
			return MakeAndCopy<ConvexShapeVisual>(source);
		}
		if (std::dynamic_pointer_cast<TextVisual>(source)) {
			return MakeAndCopy<TextVisual>(source);
		}
		if (std::dynamic_pointer_cast<SpriteVisual>(source)) {
			return MakeAndCopy<SpriteVisual>(source);
		}
		if (std::dynamic_pointer_cast<VectorArrowVisual>(source)) {
			return MakeAndCopy<VectorArrowVisual>(source);
		}
		if (std::dynamic_pointer_cast<RelativeSortingStrategy>(source)) {
			return MakeAndCopy<RelativeSortingStrategy>(source);
		}
		if (std::dynamic_pointer_cast<PhysicsBodyBehaviour>(source)) {
			return MakeAndCopy<PhysicsBodyBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<AttractiveBehaviour>(source)) {
			return MakeAndCopy<AttractiveBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<FpsCounterBehaviour>(source)) {
			return MakeAndCopy<FpsCounterBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<ButtonBehaviour>(source)) {
			return MakeAndCopy<ButtonBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<AiPlatformControllerBehaviour>(source)) {
			return MakeAndCopy<AiPlatformControllerBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<UserPlatformControllerBehaviour>(source)) {
			return MakeAndCopy<UserPlatformControllerBehaviour>(source);
		}
		if (std::dynamic_pointer_cast<SolarSystemBehaviour>(source)) {
			return MakeAndCopy<SolarSystemBehaviour>(source);
		}
		return nullptr;
	}

	std::shared_ptr<SceneNode> CloneSceneNode(const std::shared_ptr<SceneNode>& source) {
		if (!source) {
			return nullptr;
		}
		auto clone = std::make_shared<SceneNode>();
		clone->SetName(source->GetName());
		clone->SetEnabled(source->IsEnabled());
		clone->SetVisible(source->IsVisible());
		(void)CopyReflectedProperties(*source->GetLocalTransform(), *clone->GetLocalTransform());

		if (auto visualClone = std::dynamic_pointer_cast<Visual>(CloneEntity(source->GetVisual()))) {
			clone->SetVisual(std::move(visualClone));
		}
		if (auto sortingClone =
		        std::dynamic_pointer_cast<RelativeSortingStrategy>(CloneEntity(source->GetSortingStrategy()))) {
			clone->SetSortingStrategy(sortingClone);
		}
		for (const auto& behaviour : source->GetBehaviours()) {
			if (auto behaviourClone = std::dynamic_pointer_cast<Behaviour>(CloneEntity(behaviour))) {
				clone->AddBehaviour(std::move(behaviourClone));
			}
		}
		for (const auto& child : source->GetChildren()) {
			auto childClone = CloneSceneNode(child);
			if (childClone) {
				clone->AddChild(childClone);
			}
		}
		return clone;
	}

} // namespace Engine
