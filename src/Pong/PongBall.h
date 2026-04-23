#pragma once
#include "Engine/Physics/ShapeBody.h"

class PongBall final : public CircleBody
{
public:
	void update(const sf::Time& dt) override;

	float getMaxSpeed() const { return _targetSpeed; }

	float getSpeedDampingFactor() const { return _speedDampingFactor; }

	void setMaxSpeed(float maxSpeed) { _targetSpeed = maxSpeed; }

	void setSpeedDampingFactor(float speedDampingFactor) { _speedDampingFactor = speedDampingFactor; }

private:
	float _targetSpeed = 400.f;
	float _speedDampingFactor = 1.f;
};