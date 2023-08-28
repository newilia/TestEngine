#include "UserPullComponent.h"

sf::Vector2f UserPullComponent::getPullVector() const { // todo add damping
	if (auto abstractBody = dynamic_cast<AbstractBody*>(mHolder)) {
		auto pullVector = mDestPoint - abstractBody->getPhysicalComponent()->mPos - mSourcePoint;
		return pullVector;
	}
	return {};
}