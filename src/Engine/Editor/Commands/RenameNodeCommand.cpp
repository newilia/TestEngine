#include "Engine/Editor/Commands/RenameNodeCommand.h"

#include "Engine/Core/SceneNode.h"

namespace Engine::EditorCommands {

	RenameNodeCommand::RenameNodeCommand(std::shared_ptr<SceneNode> node, std::string oldName, std::string newName)
	    : _node(std::move(node)), _oldName(std::move(oldName)), _newName(std::move(newName)) {}

	bool RenameNodeCommand::Execute() {
		auto node = _node.lock();
		if (!node) {
			return false;
		}
		node->SetName(_newName);
		return true;
	}

	void RenameNodeCommand::Undo() {
		auto node = _node.lock();
		if (!node) {
			return;
		}
		node->SetName(_oldName);
	}

} // namespace Engine::EditorCommands
