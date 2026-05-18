#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <cstddef>
#include <memory>
#include <vector>

class SceneNode;

namespace Engine::EditorCommands {

	class MoveNodesInHierarchyCommand final : public Engine::IEditorCommand
	{
	public:
		struct Entry
		{
			std::weak_ptr<SceneNode> node;
			std::weak_ptr<SceneNode> oldParent;
			std::size_t oldIndex = 0;
		};

		MoveNodesInHierarchyCommand(
		    std::vector<Entry> entries, std::shared_ptr<SceneNode> newParent, std::size_t newIndex);

		bool Execute() override;
		void Undo() override;

	private:
		bool ApplyMove(
		    const std::vector<Entry>& entries, const std::shared_ptr<SceneNode>& newParent, std::size_t newIndex);

		std::vector<Entry> _entries;
		std::weak_ptr<SceneNode> _newParent;
		std::size_t _newIndex = 0;
	};

} // namespace Engine::EditorCommands
