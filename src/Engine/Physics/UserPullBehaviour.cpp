#include "UserPullBehaviour.h"

#include "Engine/SceneNode.h"

sf::Vector2f UserPullBehaviour::getPullVector() const {
	if (auto node = GetNode()) {
		if (auto* collider = node->FindShapeCollider()) {
			return _globalDestPoint - collider->GetPosGlobal() - _localSourcePoint;
		}
	}
	return {};
}
