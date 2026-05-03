#pragma once

#include <memory>

class SceneNode;

namespace Demo1 {
	/// `fieldWidth` / `fieldHeight` — внутренний прямоугольник поля; `platformWidth` / `platformHeight` — размер хитбокса платформ;
	/// `wallThickness` — глубина коллайдера стен (ось, перпендикулярная к границе поля).
	[[nodiscard]] std::shared_ptr<SceneNode> CreatePongGameNode(float fieldWidth, float fieldHeight,
	                                                            float platformWidth, float platformHeight,
	                                                            float wallThickness);
} // namespace Demo1
