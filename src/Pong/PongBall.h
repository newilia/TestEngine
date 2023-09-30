#pragma once
#include "Engine/Physics/ShapeBody.h"

class PongBall final : public CircleBody {
public:
	void update(const sf::Time& dt) override;
	float getMaxSpeed() const {	return mTargetSpeed; }
	float getSpeedDampingFactor() const { return mSpeedDampingFactor; }
	void setMaxSpeed(float maxSpeed) { mTargetSpeed = maxSpeed; }
	void setSpeedDampingFactor(float speedDampingFactor) {mSpeedDampingFactor = speedDampingFactor;	}

private:
	float mTargetSpeed = 400.f;
	float mSpeedDampingFactor = 1.f;
};