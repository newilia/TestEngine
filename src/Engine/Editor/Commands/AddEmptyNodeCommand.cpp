#include "Engine/Editor/Commands/AddEmptyNodeCommand.h"

#include "Engine/Core/SceneNode.h"

namespace Engine::EditorCommands {

	AddEmptyNodeCommand::AddEmptyNodeCommand(
	    std::shared_ptr<SceneNode> parent, std::size_t index, std::shared_ptr<SceneNode> node)
	    : _parent(std::move(parent)), _index(index), _node(std::move(node)) {}

	bool AddEmptyNodeCommand::Execute() {
		auto parent = _parent.lock();
		if (!parent || !_node) {
			return false;
		}
		parent->AddChildAt(_node, _index);
		return true;
	}

	void AddEmptyNodeCommand::Undo() {
		auto parent = _parent.lock();
		if (!parent || !_node) {
			return;
		}
		parent->RemoveChild(_node.get());
	}

} // namespace Engine::EditorCommands
