#include "Engine/Editor/Commands/CutEntityCommand.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

namespace Engine::EditorCommands {

	CutEntityCommand::CutEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
	                                   Engine::EntitySlot slot)
	    : _node(std::move(node)), _entity(std::move(entity)), _slot(slot) {}

	bool CutEntityCommand::Execute() {
		auto node = _node.lock();
		auto entity = _entity.lock();
		if (!node || !entity || _slot == Engine::EntitySlot::Transform) {
			return false;
		}
		if (_slot == Engine::EntitySlot::Visual) {
			_removedVisual = std::dynamic_pointer_cast<Visual>(entity);
			if (!node->GetVisual() || node->GetVisual().get() != _removedVisual.get()) {
				return false;
			}
			node->SetVisual(std::shared_ptr<Visual>{});
			return true;
		}
		if (_slot == Engine::EntitySlot::SortingStrategy) {
			_removedSorting = std::dynamic_pointer_cast<RelativeSortingStrategy>(entity);
			if (!node->GetSortingStrategy() || node->GetSortingStrategy().get() != _removedSorting.get()) {
				return false;
			}
			node->SetSortingStrategy(std::shared_ptr<RelativeSortingStrategy>{});
			return true;
		}
		auto behaviour = std::dynamic_pointer_cast<Behaviour>(entity);
		if (!behaviour) {
			return false;
		}
		_removedBehaviour = behaviour;
		node->RemoveBehaviour(behaviour.get());
		return true;
	}

	void CutEntityCommand::Undo() {
		auto node = _node.lock();
		if (!node) {
			return;
		}
		if (_slot == Engine::EntitySlot::Visual && _removedVisual) {
			node->SetVisual(std::move(_removedVisual));
			return;
		}
		if (_slot == Engine::EntitySlot::SortingStrategy && _removedSorting) {
			node->SetSortingStrategy(_removedSorting);
			return;
		}
		if (_slot == Engine::EntitySlot::Behaviour && _removedBehaviour) {
			node->AddBehaviour(std::move(_removedBehaviour));
			if (IsUnderActiveScene(node)) {
				node->NotifyLifecycleInitRecursive();
			}
		}
	}

} // namespace Engine::EditorCommands
