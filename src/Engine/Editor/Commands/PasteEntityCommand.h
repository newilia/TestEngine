#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/EntitySlot.h"
#include "Engine/Editor/EditorHistory.h"

#include <memory>

class Behaviour;
class SortingStrategy;
class SceneNode;
class Transform;
class Visual;

namespace Engine::EditorCommands {

	class PasteEntityCommand final : public Engine::IEditorCommand
	{
	public:
		PasteEntityCommand(std::shared_ptr<SceneNode> node, std::shared_ptr<EntityOnNode> entity,
		    Engine::EntitySlot slot, std::shared_ptr<Transform> transformEntity = nullptr);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		std::shared_ptr<EntityOnNode> _entity;
		std::shared_ptr<Transform> _transformEntity;
		Engine::EntitySlot _slot = Engine::EntitySlot::Behaviour;
		std::shared_ptr<Transform> _previousTransformState;
		std::shared_ptr<Visual> _previousVisual;
		std::shared_ptr<SortingStrategy> _previousSorting;
		std::shared_ptr<Behaviour> _replacedBehaviour;
	};

} // namespace Engine::EditorCommands
