#pragma once
#include <SFML/System/Vector2.hpp>

#include "ComponentBase.h"
#include "Utils.h"

class UserPullComponent : public ComponentBase{
public:
	UserPullComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	~UserPullComponent() override = default;

	sf::Vector2f calcPullForce() const {
		if (auto abstractBody = dynamic_cast<AbstractBody*>(mHolder)) {
			auto pullVector = mDestPoint - abstractBody->getPhysicalComponent()->mPos - mSourcePoint;
			return pullVector * mPullingStrength /** utils::length(pullVector)*/;
		}
		return {};
	}

	sf::Vector2f mSourcePoint; // local coordinate of body's pulling point
	sf::Vector2f mDestPoint; // global coordinate of pointer
	float mPullingStrength = 1000.f; //
};
