#pragma once

#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	bool IsUnderActiveScene(const std::shared_ptr<SceneNode>& node);
	bool IsNodeInSubtree(const std::shared_ptr<SceneNode>& candidate, const std::shared_ptr<SceneNode>& treeRoot);

} // namespace Engine::EditorCommands
