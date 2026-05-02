#include "PongPlatform.h"

#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"

#include <algorithm>
#include <cassert>

void ApplyPongPlatformVelocityTowardsTarget(const std::shared_ptr<SceneNode>& platformNode,
                                            const sf::Vector2f& targetPos, float speedFactor,
                                            const sf::Vector2f& velLimit) {
	if (!platformNode) {
		return;
	}
	auto* collider = platformNode->FindShapeCollider();
	if (!collider) {
		return;
	}
	auto shape = collider->GetBaseShape();
	auto vel = (targetPos - shape->getPosition()) * speedFactor;
	vel.x = std::clamp(vel.x, -velLimit.x, velLimit.x);
	vel.y = std::clamp(vel.y, -velLimit.y, velLimit.y);
	platformNode->RequireBehaviour<RigidBodyBehaviour>()->_velocity = vel;
}
