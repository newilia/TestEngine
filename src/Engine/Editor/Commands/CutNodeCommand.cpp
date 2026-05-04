#include "Engine/Editor/Commands/CutNodeCommand.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"

#include <algorithm>
#include <iterator>

namespace Engine::EditorCommands {

	CutNodeCommand::CutNodeCommand(std::shared_ptr<SceneNode> node) : _node(std::move(node)) {}

	bool CutNodeCommand::Execute() {
		auto node = _node.lock();
		if (!node) {
			return false;
		}
		auto parent = node->GetParent();
		if (!parent) {
			return false;
		}
		_parent = parent;
		const auto& children = parent->GetChildren();
		const auto it = std::find(children.begin(), children.end(), node);
		if (it == children.end()) {
			return false;
		}
		_index = static_cast<std::size_t>(std::distance(children.begin(), it));
		parent->RemoveChild(node.get());
		return true;
	}

	void CutNodeCommand::Undo() {
		auto node = _node.lock();
		auto parent = _parent.lock();
		if (!node || !parent) {
			return;
		}
		parent->AddChildAt(node, _index);
		if (IsUnderActiveScene(node)) {
			node->NotifyLifecycleInitRecursive();
		}
	}

} // namespace Engine::EditorCommands
