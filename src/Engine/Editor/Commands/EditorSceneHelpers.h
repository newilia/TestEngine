#pragma once

#include <memory>

class SceneNode;

namespace Engine::EditorCommands {

	bool IsUnderActiveScene(const std::shared_ptr<SceneNode>& node);

} // namespace Engine::EditorCommands
