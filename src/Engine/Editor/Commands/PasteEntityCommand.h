#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Editor/EditorHistory.h"
#include "Engine/Editor/SceneCloneUtils.h"

#include <memory>

class Behaviour;
class RelativeSortingStrategy;
class SceneNode;
class Transform;
class Visual;

namespace Engine::EditorCommands {

	class PasteEntityCommand final : public Engine::IEditorCommand
	{
	public:
		PasteEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
		                   Engine::EntitySlot slot);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		std::shared_ptr<EntityOnNode> _entity;
		Engine::EntitySlot _slot = Engine::EntitySlot::Behaviour;
		std::shared_ptr<Transform> _previousTransformState;
		std::shared_ptr<Visual> _previousVisual;
		std::shared_ptr<RelativeSortingStrategy> _previousSorting;
		std::shared_ptr<Behaviour> _replacedBehaviour;
	};

} // namespace Engine::EditorCommands
