#include "Engine/Editor/Commands/PasteNodeCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"

namespace Engine::EditorCommands {

	PasteNodeCommand::PasteNodeCommand(std::shared_ptr<SceneNode> parent, std::shared_ptr<SceneNode> node)
	    : _parent(std::move(parent)), _node(std::move(node)) {}

	bool PasteNodeCommand::Execute() {
		auto parent = _parent.lock();
		if (!parent || !_node) {
			return false;
		}
		parent->AddChild(_node);
		if (IsUnderActiveScene(_node)) {
			_node->NotifyLifecycleInitRecursive();
		}
		return true;
	}

	void PasteNodeCommand::Undo() {
		auto parent = _parent.lock();
		if (!parent || !_node) {
			return;
		}
		parent->RemoveChild(_node.get());
	}

} // namespace Engine::EditorCommands
