#include "Engine/Editor/Commands/MoveNodesInHierarchyCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

namespace Engine::EditorCommands {
	namespace {

		struct ResolvedEntry
		{
			std::shared_ptr<SceneNode> node;
			std::shared_ptr<SceneNode> oldParent;
			std::size_t oldIndex = 0;
		};

		std::vector<ResolvedEntry> ResolveEntries(const std::vector<MoveNodesInHierarchyCommand::Entry>& entries) {
			std::vector<ResolvedEntry> resolved;
			resolved.reserve(entries.size());
			for (const auto& entry : entries) {
				auto node = entry.node.lock();
				auto oldParent = entry.oldParent.lock();
				if (!node || !oldParent) {
					return {};
				}
				resolved.push_back({std::move(node), std::move(oldParent), entry.oldIndex});
			}
			return resolved;
		}

		std::size_t CountIndicesBelow(std::size_t insertIndex, const std::vector<ResolvedEntry>& entries,
		    const std::shared_ptr<SceneNode>& parent) {
			std::size_t count = 0;
			for (const auto& entry : entries) {
				if (entry.oldParent == parent && entry.oldIndex < insertIndex) {
					++count;
				}
			}
			return count;
		}

		void RemoveEntriesFromParents(const std::vector<ResolvedEntry>& entries) {
			std::map<SceneNode*, std::pair<std::shared_ptr<SceneNode>, std::vector<std::pair<std::size_t, SceneNode*>>>>
			    byParent;
			for (const auto& entry : entries) {
				auto& slot = byParent[entry.oldParent.get()];
				slot.first = entry.oldParent;
				slot.second.emplace_back(entry.oldIndex, entry.node.get());
			}
			for (auto& [parentPtr, parentAndRemovals] : byParent) {
				(void)parentPtr;
				auto& [parent, removals] = parentAndRemovals;
				std::sort(removals.begin(), removals.end(), [](const auto& a, const auto& b) {
					return a.first > b.first;
				});
				for (const auto& [index, childPtr] : removals) {
					(void)index;
					parent->RemoveChild(childPtr);
				}
			}
		}

		void InsertEntriesAt(const std::vector<ResolvedEntry>& entries, const std::shared_ptr<SceneNode>& newParent,
		    std::size_t insertIndex) {
			std::size_t index = insertIndex;
			for (const auto& entry : entries) {
				newParent->AddChildAt(entry.node, index);
				++index;
				if (IsUnderActiveScene(entry.node)) {
					entry.node->NotifyLifecycleInitRecursive();
				}
			}
		}

		void RestoreEntriesToOriginalParents(std::vector<ResolvedEntry> entries) {
			std::stable_sort(entries.begin(), entries.end(), [](const ResolvedEntry& a, const ResolvedEntry& b) {
				if (a.oldParent != b.oldParent) {
					return a.oldParent.get() < b.oldParent.get();
				}
				return a.oldIndex < b.oldIndex;
			});
			for (const auto& entry : entries) {
				entry.oldParent->AddChildAt(entry.node, entry.oldIndex);
				if (IsUnderActiveScene(entry.node)) {
					entry.node->NotifyLifecycleInitRecursive();
				}
			}
		}

	} // namespace

	MoveNodesInHierarchyCommand::MoveNodesInHierarchyCommand(
	    std::vector<Entry> entries, std::shared_ptr<SceneNode> newParent, std::size_t newIndex)
	    : _entries(std::move(entries)), _newParent(std::move(newParent)), _newIndex(newIndex) {}

	bool MoveNodesInHierarchyCommand::ApplyMove(
	    const std::vector<Entry>& entries, const std::shared_ptr<SceneNode>& newParent, std::size_t newIndex) {
		auto resolved = ResolveEntries(entries);
		if (resolved.empty() || !newParent) {
			return false;
		}

		struct SavedWorldPos
		{
			std::weak_ptr<SceneNode> node;
			sf::Vector2f position{};
		};

		std::vector<SavedWorldPos> savedWorldPositions;
		savedWorldPositions.reserve(resolved.size());
		for (const auto& entry : resolved) {
			if (entry.oldParent != newParent) {
				savedWorldPositions.push_back({entry.node, Utils::GetWorldPos(entry.node)});
			}
		}

		std::size_t insertIndex = newIndex;
		if (const std::size_t shift = CountIndicesBelow(insertIndex, resolved, newParent); shift > 0) {
			insertIndex -= shift;
		}

		RemoveEntriesFromParents(resolved);
		InsertEntriesAt(resolved, newParent, insertIndex);

		for (const auto& saved : savedWorldPositions) {
			if (auto node = saved.node.lock()) {
				Utils::SetLocalPosToWorld(node, saved.position);
			}
		}
		return true;
	}

	bool MoveNodesInHierarchyCommand::Execute() {
		auto newParent = _newParent.lock();
		if (!newParent) {
			return false;
		}
		return ApplyMove(_entries, newParent, _newIndex);
	}

	void MoveNodesInHierarchyCommand::Undo() {
		auto newParent = _newParent.lock();
		if (!newParent) {
			return;
		}

		std::vector<ResolvedEntry> onNewParent;
		onNewParent.reserve(_entries.size());
		std::size_t index = _newIndex;
		for (const auto& entry : _entries) {
			auto node = entry.node.lock();
			if (!node) {
				return;
			}
			onNewParent.push_back({node, newParent, index});
			++index;
		}

		RemoveEntriesFromParents(onNewParent);

		auto original = ResolveEntries(_entries);
		if (original.empty()) {
			return;
		}
		RestoreEntriesToOriginalParents(std::move(original));
	}

} // namespace Engine::EditorCommands
