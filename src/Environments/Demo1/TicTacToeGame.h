#pragma once

#include <memory>

class SceneNode;

namespace Demo1 {
	[[nodiscard]] std::shared_ptr<SceneNode> CreateTicTacToeGameNode();
} // namespace Demo1
