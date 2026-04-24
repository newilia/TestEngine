#include "UserPullBehaviour.h"

#include "Engine/Core/SceneNode.h"

sf::Vector2f UserPullBehaviour::GetPullVector() const {
	if (auto node = GetNode()) {
		if (auto* collider = node->FindShapeCollider()) {
			return _globalDestPoint - collider->GetPosGlobal() - _localSourcePoint;
		}
	}
	return {};
}
