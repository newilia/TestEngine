#pragma once
#include "ComponentBase.h"


class PhysicalComponent : public ComponentBase {
public:
	PhysicalComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	float mMass = 1.f;
	sf::Vector2f mPos;
	sf::Vector2f mVelocity;

	float mAngle = 0.f;
	float mAngularSpeed = 0.f;

	sf::Vector2f mForce;
	float mBounce = 1.f;
	float mFriction = 0.5f;
};
