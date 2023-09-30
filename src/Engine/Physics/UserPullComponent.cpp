#include "UserPullComponent.h"

#include "Engine/Physics/AbstractBody.h"

sf::Vector2f UserPullComponent::getPullVector() const {
	if (auto abstractBody = dynamic_cast<AbstractBody*>(mHolder)) {
		auto pullVector = mGlobalDestPoint - abstractBody->getPosGlobal() - mLocalSourcePoint;
		return pullVector;
	}
	return {};
}
