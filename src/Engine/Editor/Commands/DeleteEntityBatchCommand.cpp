#include "Engine/Editor/Commands/DeleteEntityBatchCommand.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

namespace Engine::EditorCommands {

	DeleteEntityBatchCommand::DeleteEntityBatchCommand(std::vector<Entry> entries) : _entries(std::move(entries)) {}

	bool DeleteEntityBatchCommand::Execute() {
		for (Entry& entry : _entries) {
			auto node = entry.node.lock();
			auto entity = entry.entity.lock();
			if (!node || !entity || entry.slot == Engine::EntitySlot::Transform) {
				return false;
			}
			if (entry.slot == Engine::EntitySlot::Visual) {
				entry.removedVisual = std::dynamic_pointer_cast<Visual>(entity);
				if (!node->GetVisual() || node->GetVisual().get() != entry.removedVisual.get()) {
					return false;
				}
				node->SetVisual(std::shared_ptr<Visual>{});
				continue;
			}
			if (entry.slot == Engine::EntitySlot::SortingStrategy) {
				entry.removedSorting = std::dynamic_pointer_cast<SortingStrategy>(entity);
				if (!node->GetSortingStrategy() || node->GetSortingStrategy().get() != entry.removedSorting.get()) {
					return false;
				}
				node->SetSortingStrategy(std::shared_ptr<SortingStrategy>{});
				continue;
			}
			auto behaviour = std::dynamic_pointer_cast<Behaviour>(entity);
			if (!behaviour) {
				return false;
			}
			entry.removedBehaviour = behaviour;
			node->RemoveBehaviour(behaviour.get());
		}
		return true;
	}

	void DeleteEntityBatchCommand::Undo() {
		for (auto it = _entries.rbegin(); it != _entries.rend(); ++it) {
			Entry& entry = *it;
			auto node = entry.node.lock();
			if (!node) {
				continue;
			}
			if (entry.slot == Engine::EntitySlot::Visual && entry.removedVisual) {
				node->SetVisual(std::move(entry.removedVisual));
				continue;
			}
			if (entry.slot == Engine::EntitySlot::SortingStrategy && entry.removedSorting) {
				node->SetSortingStrategy(entry.removedSorting);
				continue;
			}
			if (entry.slot == Engine::EntitySlot::Behaviour && entry.removedBehaviour) {
				node->AddBehaviour(std::move(entry.removedBehaviour));
				if (IsUnderActiveScene(node)) {
					node->NotifyLifecycleInitRecursive();
				}
			}
		}
	}

} // namespace Engine::EditorCommands
