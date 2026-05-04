#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Editor/EditorHistory.h"
#include "Engine/Editor/SceneCloneUtils.h"

#include <memory>

class Behaviour;
class RelativeSortingStrategy;
class SceneNode;
class Visual;

namespace Engine::EditorCommands {

	class CutEntityCommand final : public Engine::IEditorCommand
	{
	public:
		CutEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
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
