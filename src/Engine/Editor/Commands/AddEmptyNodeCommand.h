#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <cstddef>
#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	class AddEmptyNodeCommand final : public Engine::IEditorCommand
	{
	public:
		AddEmptyNodeCommand(std::shared_ptr<SceneNode> parent, std::size_t index, std::shared_ptr<SceneNode> node);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _parent;
		std::size_t _index = 0;
		std::shared_ptr<SceneNode> _node;
	};

} // namespace Engine::EditorCommands
