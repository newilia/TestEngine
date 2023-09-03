#include "UserPullComponent.h"

sf::Vector2f UserPullComponent::getPullVector() const { // todo add damping
	if (auto abstractBody = dynamic_cast<AbstractBody*>(mHolder)) {
		auto pullVector = mGlobalDestPoint - abstractBody->getPhysicalComponent()->mPos - mLocalSourcePoint;
		return pullVector;
	}
	return {};
}