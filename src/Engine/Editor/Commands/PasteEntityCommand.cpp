#include "Engine/Editor/Commands/PasteEntityCommand.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <typeinfo>

namespace Engine::EditorCommands {

	PasteEntityCommand::PasteEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
	                                       Engine::EntitySlot slot)
	    : _node(std::move(node)), _entity(std::move(entity)), _slot(slot) {}

	bool PasteEntityCommand::Execute() {
		auto node = _node.lock();
		if (!node || !_entity) {
			return false;
		}
		if (_slot == Engine::EntitySlot::Transform) {
			auto transformEntity = std::dynamic_pointer_cast<Transform>(_entity);
			if (!transformEntity) {
				return false;
			}
			if (!_previousTransformState) {
				_previousTransformState =
				    std::dynamic_pointer_cast<Transform>(Engine::CloneEntity(node->GetLocalTransform()));
			}
			(void)Engine::CopyReflectedProperties(*transformEntity, *node->GetLocalTransform());
			return true;
		}
		if (_slot == Engine::EntitySlot::Visual) {
			if (!_previousVisual) {
				_previousVisual = node->GetVisual();
			}
			auto visualEntity = std::dynamic_pointer_cast<Visual>(_entity);
			if (!visualEntity) {
				return false;
			}
			node->SetVisual(std::move(visualEntity));
			return true;
		}
		if (_slot == Engine::EntitySlot::SortingStrategy) {
			if (!_previousSorting) {
				_previousSorting = node->GetSortingStrategy();
			}
			auto sortingEntity = std::dynamic_pointer_cast<RelativeSortingStrategy>(_entity);
			if (!sortingEntity) {
				return false;
			}
			node->SetSortingStrategy(sortingEntity);
			return true;
		}

		auto behaviourEntity = std::dynamic_pointer_cast<Behaviour>(_entity);
		if (!behaviourEntity) {
			return false;
		}
		if (!_replacedBehaviour) {
			for (const auto& behaviour : node->GetBehaviours()) {
				if (behaviour && typeid(*behaviour) == typeid(*behaviourEntity)) {
					_replacedBehaviour = behaviour;
					break;
				}
			}
		}
		if (_replacedBehaviour) {
			node->RemoveBehaviour(_replacedBehaviour.get());
		}
		node->AddBehaviour(behaviourEntity);
		if (IsUnderActiveScene(node)) {
			node->NotifyLifecycleInitRecursive();
		}
		return true;
	}

	void PasteEntityCommand::Undo() {
		auto node = _node.lock();
		if (!node) {
			return;
		}
		if (_slot == Engine::EntitySlot::Transform) {
			if (_previousTransformState) {
				(void)Engine::CopyReflectedProperties(*_previousTransformState, *node->GetLocalTransform());
			}
			return;
		}
		if (_slot == Engine::EntitySlot::Visual) {
			node->SetVisual(std::move(_previousVisual));
			return;
		}
		if (_slot == Engine::EntitySlot::SortingStrategy) {
			node->SetSortingStrategy(_previousSorting);
			return;
		}
		auto behaviourEntity = std::dynamic_pointer_cast<Behaviour>(_entity);
		if (behaviourEntity) {
			node->RemoveBehaviour(behaviourEntity.get());
		}
		if (_replacedBehaviour) {
			node->AddBehaviour(std::move(_replacedBehaviour));
			if (IsUnderActiveScene(node)) {
				node->NotifyLifecycleInitRecursive();
			}
		}
	}

} // namespace Engine::EditorCommands
