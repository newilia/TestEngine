#pragma once

class PhysicalObject;

class PhysicalComponent : public ComponentBase {
public:

private:
	float mMass = 1.f;
	sf::Vector2f mPos;
	sf::Vector2f mSpeed;
	sf::Vector2f mForce;
	float mRestitution = 0.5f;
	float mFriction = 0.5f;
};
