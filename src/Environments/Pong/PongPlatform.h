#pragma once

#include "Engine/Core/SceneNode.h"

#include <SFML/System/Vector2.hpp>

/// Sets rigid-body velocity toward `targetPos` (used by user + AI platform behaviours).
void ApplyPongPlatformVelocityTowardsTarget(const std::shared_ptr<SceneNode>& platform, const sf::Vector2f& targetPos,
                                            float speedFactor, const sf::Vector2f& velLimit);

/// Keeps the platform collider inside the playfield horizontally and inside a vertical strip of
/// `0.2 * window height` measured from the top or bottom field edge toward the center.
void ClampPongPlatformToPlayfield(const std::shared_ptr<SceneNode>& platformNode, bool isBottomPlayer);

/// Clamps a desired shape position (collider origin) to the same bounds using current rotation/bbox extents.
void ClampPongPlatformDesiredCenter(sf::Vector2f& center, bool isBottomPlayer,
                                    const std::shared_ptr<SceneNode>& platformNode);
