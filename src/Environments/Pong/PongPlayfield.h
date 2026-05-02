#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

/// Vertical travel zone for each paddle: strip from the playfield edge toward the center, expressed as a
/// fraction of the **window** height (same value for top and bottom paddles).
inline constexpr float kPongPlatformVerticalStripScreenFraction = 0.3f;

/// Inner playfield rectangle in window coordinates (symmetric margins from `PongPlayfield.cpp`).
sf::FloatRect GetPongPlayfieldRect();

sf::Vector2f GetPongWindowSize();
