#pragma once

#include "Engine/Core/SceneNode.h"

#include <SFML/System/Vector2.hpp>

/// Sets rigid-body velocity toward `targetPos` (used by user + AI platform behaviours).
void ApplyPongPlatformVelocityTowardsTarget(const std::shared_ptr<SceneNode>& platform, const sf::Vector2f& targetPos,
                                            float speedFactor, const sf::Vector2f& velLimit);
