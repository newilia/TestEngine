#pragma once

#include "Engine/Physics/PhysicalBehaviour.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/System/Time.hpp>

#include <memory>

namespace sf {
class CircleShape;
}

/// Игровой шарик: коллайдер и визуал задаются поведениями на одной ноде.
class PongBall : public std::enable_shared_from_this<PongBall>
{
public:
	explicit PongBall(std::shared_ptr<SceneNode> node);

	void setupBehaviours();

	std::shared_ptr<SceneNode> GetNode() const { return _node; }

	sf::CircleShape* GetShape() const;

	std::shared_ptr<PhysicalBehaviour> GetPhysicalComponent() const { return _node->GetPhysicalComponent(); }

	float getMaxSpeed() const { return _targetSpeed; }

	float getSpeedDampingFactor() const { return _speedDampingFactor; }

	void setMaxSpeed(float maxSpeed) { _targetSpeed = maxSpeed; }

	void setSpeedDampingFactor(float speedDampingFactor) { _speedDampingFactor = speedDampingFactor; }

private:
	std::shared_ptr<SceneNode> _node;
	float _targetSpeed = 400.f;
	float _speedDampingFactor = 1.f;
};
