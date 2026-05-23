#pragma once

#include "Engine/Editor/EditorHistory.h"

#include <memory>
#include <string>

class SceneNode;

namespace Engine::EditorCommands {

	class RenameNodeCommand final : public Engine::IEditorCommand
	{
	public:
		RenameNodeCommand(std::shared_ptr<SceneNode> node, std::string oldName, std::string newName);

		bool Execute() override;
		void Undo() override;

	private:
		std::weak_ptr<SceneNode> _node;
		std::string _oldName;
		std::string _newName;
	};

} // namespace Engine::EditorCommands
