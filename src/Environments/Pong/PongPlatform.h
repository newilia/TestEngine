#pragma once

#include "Engine/Core/SceneNode.h"

#include <SFML/System/Vector2.hpp>

#include <memory>

/// Sets rigid-body velocity toward `targetPos` (used by user + AI platform behaviours).
void ApplyPongPlatformVelocityTowardsTarget(const std::shared_ptr<SceneNode>& platform, const sf::Vector2f& targetPos,
                                            float speedFactor, const sf::Vector2f& velLimit);

/// Keeps the platform collider inside movement bounds. When `movementBounds` locks to a rectangle node, clamps to
/// its world axis-aligned bounds. Otherwise uses playfield rect + vertical strip.
void ClampPongPlatformToPlayfield(const std::shared_ptr<SceneNode>& platformNode, bool isBottomPlayer,
                                  const std::weak_ptr<SceneNode>& movementBounds = {});

/// Clamps a desired shape position (collider origin) to the same bounds using current rotation/bbox extents.
void ClampPongPlatformDesiredCenter(sf::Vector2f& center, bool isBottomPlayer,
                                    const std::shared_ptr<SceneNode>& platformNode,
                                    const std::weak_ptr<SceneNode>& movementBounds = {});
