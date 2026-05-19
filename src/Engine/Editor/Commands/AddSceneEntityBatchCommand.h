#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/EntitySlot.h"
#include "Engine/Editor/EditorHistory.h"

#include <memory>
#include <vector>

class RelativeSortingStrategy;
class SceneNode;
class Visual;

namespace Engine::EditorCommands {

	class AddSceneEntityBatchCommand final : public Engine::IEditorCommand
	{
	public:
		struct Entry
		{
			std::weak_ptr<SceneNode> node;
			std::shared_ptr<EntityOnNode> entity;
			Engine::EntitySlot slot = Engine::EntitySlot::Behaviour;
			std::shared_ptr<Visual> previousVisual;
			std::shared_ptr<RelativeSortingStrategy> previousSorting;
		};

		explicit AddSceneEntityBatchCommand(std::vector<Entry> entries);

		bool Execute() override;
		void Undo() override;

	private:
		std::vector<Entry> _entries;
	};

} // namespace Engine::EditorCommands
