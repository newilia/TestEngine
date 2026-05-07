#pragma once

#include "Engine/Core/SceneNode.h"

#include <SFML/System/Time.hpp>

#include <memory>

namespace sf {
	class CircleShape;
}

// todo remove
class PongBall : public std::enable_shared_from_this<PongBall>
{
public:
	explicit PongBall(std::shared_ptr<SceneNode> node);
	std::shared_ptr<SceneNode> GetNode() const;
	void SetupBehaviours();

	float GetMaxSpeed() const;
	void SetMaxSpeed(float maxSpeed);

	float GetSpeedDampingFactor() const;
	void SetSpeedDampingFactor(float speedDampingFactor);

private:
	std::shared_ptr<SceneNode> _node;
	float _targetSpeed = 400.f;
	float _speedDampingFactor = 1.f;
};
