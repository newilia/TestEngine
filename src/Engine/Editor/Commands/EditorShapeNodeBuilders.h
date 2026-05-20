#pragma once

#include "Engine/Core/ColorUtils.h"

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>

class SceneNode;

namespace Engine::EditorCommands {

	[[nodiscard]] std::shared_ptr<SceneNode> BuildRectangleShapeNode(
	    sf::Vector2f centerWorld, sf::Vector2f size, bool attachPhysics, Utils::HsvShapeColors colors);

	[[nodiscard]] std::shared_ptr<SceneNode> BuildCircleShapeNode(
	    sf::Vector2f centerWorld, float radius, bool attachPhysics, Utils::HsvShapeColors colors);

	[[nodiscard]] std::shared_ptr<SceneNode> BuildPolygonShapeNode(sf::Vector2f centerWorld,
	    std::vector<sf::Vector2f> localPoints, bool attachPhysics, Utils::HsvShapeColors colors);

} // namespace Engine::EditorCommands
