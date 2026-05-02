#include "PongPlatform.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "PongPlayfield.h"

#include <algorithm>

namespace {

	void ComputePlatformCenterBounds(const sf::Vector2f& shapePos, const sf::FloatRect& bb, bool isBottomPlayer,
	                                 const sf::FloatRect& field, float screenHeight, float& minX, float& maxX,
	                                 float& minY, float& maxY) {
		const float oxl = field.position.x;
		const float oxr = field.position.x + field.size.x;
		const float oyt = field.position.y;
		const float oyb = field.position.y + field.size.y;
		const float stripH = kPongPlatformVerticalStripScreenFraction * screenHeight;

		const float dLeft = shapePos.x - bb.position.x;
		const float dRight = bb.position.x + bb.size.x - shapePos.x;
		const float dTop = shapePos.y - bb.position.y;
		const float dBot = bb.position.y + bb.size.y - shapePos.y;

		minX = oxl + dLeft;
		maxX = oxr - dRight;

		if (isBottomPlayer) {
			const float stripTop = oyb - stripH;
			minY = stripTop + dTop;
			maxY = oyb - dBot;
		}
		else {
			const float stripBot = oyt + stripH;
			minY = oyt + dTop;
			maxY = stripBot - dBot;
		}

		if (minX > maxX) {
			const float mid = (minX + maxX) * 0.5f;
			minX = maxX = mid;
		}
		if (minY > maxY) {
			const float mid = isBottomPlayer ? (oyb - stripH * 0.5f) : (oyt + stripH * 0.5f);
			minY = maxY = mid;
		}
	}
} // namespace

void ApplyPongPlatformVelocityTowardsTarget(const std::shared_ptr<SceneNode>& platformNode,
                                            const sf::Vector2f& targetPos, float speedFactor,
                                            const sf::Vector2f& velLimit) {
	if (!platformNode) {
		return;
	}
	auto* collider = platformNode->FindPhysicsBody();
	if (!collider) {
		return;
	}
	auto shape = collider->GetShape();
	auto vel = (targetPos - shape->getPosition()) * speedFactor;
	vel.x = std::clamp(vel.x, -velLimit.x, velLimit.x);
	vel.y = std::clamp(vel.y, -velLimit.y, velLimit.y);
	platformNode->RequireBehaviour<PhysicsBodyBehaviour>()->_velocity = vel;
}

void ClampPongPlatformDesiredCenter(sf::Vector2f& center, bool isBottomPlayer,
                                    const std::shared_ptr<SceneNode>& platformNode) {
	if (!platformNode) {
		return;
	}
	auto* collider = platformNode->FindPhysicsBody();
	if (!collider) {
		return;
	}
	const sf::Vector2f shapePos = collider->GetPosGlobal();
	const sf::FloatRect bb = collider->GetBbox();
	const sf::FloatRect field = GetPongPlayfieldRect();
	const float screenH = GetPongWindowSize().y;

	float minX, maxX, minY, maxY;
	ComputePlatformCenterBounds(shapePos, bb, isBottomPlayer, field, screenH, minX, maxX, minY, maxY);

	center.x = std::clamp(center.x, minX, maxX);
	center.y = std::clamp(center.y, minY, maxY);
}

void ClampPongPlatformToPlayfield(const std::shared_ptr<SceneNode>& platformNode, bool isBottomPlayer) {
	if (!platformNode) {
		return;
	}
	auto* collider = platformNode->FindPhysicsBody();
	if (!collider) {
		return;
	}
	sf::Vector2f pos = collider->GetPosGlobal();
	ClampPongPlatformDesiredCenter(pos, isBottomPlayer, platformNode);
	platformNode->SetPosGlobal(pos);
}
