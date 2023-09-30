#pragma once
#include "Engine/ComponentBase.h"
#include <bitset>

struct IntersectionDetails;
class AbstractBody;

class PhysicalComponent : public ComponentBase {
public:
	PhysicalComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	float mMass = 1.f; // kg
	sf::Vector2f mVelocity;

	float mAngle = 0.f; // radians
	float mAngularSpeed = 0.f; // rad/s
	
	float mRestitution = 0.5f; // 0 to 1
	float mFriction = 0.5f;

	void setImmovable() { mMass = std::numeric_limits<float>::infinity(); }
	bool isImmovable() const { return mMass == std::numeric_limits<float>::infinity(); }
};
