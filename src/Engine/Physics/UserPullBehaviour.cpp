#include "UserPullBehaviour.h"

#include "AbstractBody.h"

sf::Vector2f UserPullBehaviour::getPullVector() const {
	if (auto node = GetNode()) {
		if (auto abstractBody = std::dynamic_pointer_cast<AbstractBody>(node)) {
			return _globalDestPoint - abstractBody->GetPosGlobal() - _localSourcePoint;
		}
	}
	return {};
}
