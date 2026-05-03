#pragma once

#include <memory>

class SceneNode;

namespace Demo1 {
	std::shared_ptr<SceneNode> CreatePongGameNode(float fieldWidth, float fieldHeight, float platformWidth,
	                                              float platformHeight, float wallThickness, float ballRadius);
} // namespace Demo1
