#pragma once

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

	void SetupBehaviours();

	std::shared_ptr<SceneNode> GetNode() const { return _node; }

	sf::CircleShape* GetShape() const;

	float GetMaxSpeed() const { return _targetSpeed; }

	float GetSpeedDampingFactor() const { return _speedDampingFactor; }

	void SetMaxSpeed(float maxSpeed) { _targetSpeed = maxSpeed; }

	void SetSpeedDampingFactor(float speedDampingFactor) { _speedDampingFactor = speedDampingFactor; }

private:
	std::shared_ptr<SceneNode> _node;
	float _targetSpeed = 400.f;
	float _speedDampingFactor = 1.f;
};
