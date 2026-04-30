#pragma once

#include "Engine/Core/Scene.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

/// Top-most tap target, then first physics body under the point (same iteration as legacy pull).
[[nodiscard]] std::shared_ptr<SceneNode> PickSceneNodeAt(const std::shared_ptr<Scene>& scene,
                                                         sf::Vector2f windowPosition);
