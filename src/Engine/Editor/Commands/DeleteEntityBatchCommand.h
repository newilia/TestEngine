#pragma once

#include "Engine/Core/EntitySlot.h"
#include "Engine/Editor/EditorHistory.h"

#include <memory>
#include <vector>

class Behaviour;
class EntityOnNode;
class RelativeSortingStrategy;
class SceneNode;
class Visual;

namespace Engine::EditorCommands {

	class DeleteEntityBatchCommand final : public Engine::IEditorCommand
	{
	public:
		struct Entry
		{
			std::weak_ptr<SceneNode> node;
			std::weak_ptr<EntityOnNode> entity;
			Engine::EntitySlot slot = Engine::EntitySlot::Behaviour;
			std::shared_ptr<Visual> removedVisual;
			std::shared_ptr<RelativeSortingStrategy> removedSorting;
			std::shared_ptr<Behaviour> removedBehaviour;
		};

		explicit DeleteEntityBatchCommand(std::vector<Entry> entries);

		bool Execute() override;
		void Undo() override;

	private:
		std::vector<Entry> _entries;
	};

} // namespace Engine::EditorCommands
