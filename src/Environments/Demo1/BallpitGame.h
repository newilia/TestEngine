#pragma once

#include <SFML/Graphics.hpp>

#include <memory>

class SceneNode;

namespace Demo1 {
	std::shared_ptr<SceneNode> CreateBallpitGameNode(float aquariumWidth, float aquariumHeight, float wallThickness,
	                                                 float baseBallRadius, float ballRadiusVariability,
	                                                 sf::Color baseBallColor, float ballColorVariability, int ballCount,
	                                                 float ballRestitution, float wallRestitution);
} // namespace Demo1
