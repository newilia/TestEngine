#pragma once

#include "Engine/Core/Scene.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

/// Top-most tap target, then first physics body under the point (same iteration as legacy pull).
/// `worldPoint` — координаты в пространстве сцены (`RenderWindow::mapPixelToCoords`).
[[nodiscard]] std::shared_ptr<SceneNode> PickSceneNodeAt(const std::shared_ptr<Scene>& scene,
                                                         const sf::Vector2f& worldPoint);
