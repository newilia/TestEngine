#include "Engine/Editor/Commands/AddEmptyNodeCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Editor/Editor.h"

namespace Engine::EditorCommands {

	AddEmptyNodeCommand::AddEmptyNodeCommand(std::shared_ptr<SceneNode> parent, const std::size_t index)
	    : _parent(std::move(parent)), _index(index) {}

	bool AddEmptyNodeCommand::Execute() {
		auto parent = _parent.lock();
		if (!parent) {
			return false;
		}
		auto node = SceneNode::Create();
		parent->AddChildAt(node, _index);
		if (IsUnderActiveScene(node)) {
			node->NotifyLifecycleInitRecursive();
		}
		_node = node;
		Engine::Editor::GetInstance().SetSelectedNode(node);
		return true;
	}

	void AddEmptyNodeCommand::Undo() {
		auto node = _node.lock();
		auto parent = _parent.lock();
		if (!node || !parent) {
			return;
		}
		parent->RemoveChild(node.get());
	}

} // namespace Engine::EditorCommands
