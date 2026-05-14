#include "Engine/Editor/Commands/AddSceneEntityBatchCommand.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

namespace Engine::EditorCommands {

	AddSceneEntityBatchCommand::AddSceneEntityBatchCommand(std::vector<Entry> entries) : _entries(std::move(entries)) {}

	bool AddSceneEntityBatchCommand::Execute() {
		for (Entry& entry : _entries) {
			auto node = entry.node.lock();
			if (!node || !entry.entity) {
				return false;
			}
			if (entry.slot == Engine::EntitySlot::Visual) {
				entry.previousVisual = node->GetVisual();
				auto visualEntity = std::dynamic_pointer_cast<Visual>(entry.entity);
				if (!visualEntity) {
					return false;
				}
				node->SetVisual(std::move(visualEntity));
				continue;
			}
			if (entry.slot == Engine::EntitySlot::SortingStrategy) {
				entry.previousSorting = node->GetSortingStrategy();
				auto sortingEntity = std::dynamic_pointer_cast<RelativeSortingStrategy>(entry.entity);
				if (!sortingEntity) {
					return false;
				}
				node->SetSortingStrategy(sortingEntity);
				continue;
			}
			if (entry.slot == Engine::EntitySlot::Behaviour) {
				auto behaviourEntity = std::dynamic_pointer_cast<Behaviour>(entry.entity);
				if (!behaviourEntity) {
					return false;
				}
				node->AddBehaviour(behaviourEntity);
				if (IsUnderActiveScene(node)) {
					node->NotifyLifecycleInitRecursive();
				}
				continue;
			}
			return false;
		}
		return true;
	}

	void AddSceneEntityBatchCommand::Undo() {
		for (auto it = _entries.rbegin(); it != _entries.rend(); ++it) {
			Entry& entry = *it;
			auto node = entry.node.lock();
			if (!node) {
				continue;
			}
			if (entry.slot == Engine::EntitySlot::Visual) {
				node->SetVisual(std::move(entry.previousVisual));
				continue;
			}
			if (entry.slot == Engine::EntitySlot::SortingStrategy) {
				node->SetSortingStrategy(entry.previousSorting);
				continue;
			}
			if (entry.slot == Engine::EntitySlot::Behaviour) {
				auto behaviourEntity = std::dynamic_pointer_cast<Behaviour>(entry.entity);
				if (behaviourEntity) {
					node->RemoveBehaviour(behaviourEntity.get());
				}
			}
		}
	}

} // namespace Engine::EditorCommands
