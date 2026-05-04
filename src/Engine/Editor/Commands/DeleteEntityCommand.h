#pragma once

#include "Engine/Editor/EditorHistory.h"
#include "Engine/Editor/SceneCloneUtils.h"

#include <memory>

class Behaviour;
class EntityOnNode;
class RelativeSortingStrategy;
class SceneNode;
class Visual;

namespace Engine::EditorCommands {

	class DeleteEntityCommand final : public Engine::IEditorCommand
	{
	public:
		DeleteEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
		                    Engine::EntitySlot slot);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		std::weak_ptr<EntityOnNode> _entity;
		Engine::EntitySlot _slot = Engine::EntitySlot::Behaviour;
		std::shared_ptr<Visual> _removedVisual;
		std::shared_ptr<RelativeSortingStrategy> _removedSorting;
		std::shared_ptr<Behaviour> _removedBehaviour;
	};

} // namespace Engine::EditorCommands
