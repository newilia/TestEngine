#pragma once
#include "Engine/ComponentBase.h"

#include <bitset>

struct IntersectionDetails;
class AbstractBody;

class PhysicalComponent : public ComponentBase
{
public:
	PhysicalComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	float _mass = 1.f; // kg
	sf::Vector2f _velocity;

	float _angle = 0.f;        // radians
	float _angularSpeed = 0.f; // rad/s

	float _restitution = 0.5f; // 0 to 1
	float _friction = 0.5f;

	void setImmovable() { _mass = std::numeric_limits<float>::infinity(); }

	bool isImmovable() const { return _mass == std::numeric_limits<float>::infinity(); }
};
