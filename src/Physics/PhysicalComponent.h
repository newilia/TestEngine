#pragma once
#include "ComponentBase.h"

class PhysicalComponent : public ComponentBase {
public:
	PhysicalComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	float mMass = 1.f; // kg
	sf::Vector2f mPos; // global position of center of the mass
	sf::Vector2f mVelocity;

	float mAngle = 0.f; // radians
	float mAngularSpeed = 0.f; // rad/s
	
	float mRestitution = 1.f; // 0 to 1
	float mFriction = 0.5f;

	bool isImmovable() const { return mMass == std::numeric_limits<float>::infinity(); }
};
