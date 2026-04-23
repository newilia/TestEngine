#include "UserPullComponent.h"

#include "Engine/Physics/AbstractBody.h"

sf::Vector2f UserPullComponent::getPullVector() const {
	if (auto abstractBody = dynamic_cast<AbstractBody*>(_holder)) {
		auto pullVector = _globalDestPoint - abstractBody->getPosGlobal() - _localSourcePoint;
		return pullVector;
	}
	return {};
}
