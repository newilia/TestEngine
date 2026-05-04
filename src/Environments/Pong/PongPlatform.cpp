#include "PongPlatform.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/RectangleShapeVisual.h"
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

	sf::FloatRect GetRectangleNodeWorldAabb(const SceneNode& n) {
		auto rv = std::dynamic_pointer_cast<RectangleShapeVisual>(n.GetVisual());
		if (!rv) {
			return {};
		}
		const sf::RectangleShape* sh = rv->GetShape();
		sf::Transform t = n.GetWorldTransform();
		t *= sh->getTransform();
		return Utils::AxisAlignedBoundsAfterTransform(t, sh->getLocalBounds());
	}

	void ComputeClampFromAxisAlignedRegion(const sf::Vector2f& shapePos, const sf::FloatRect& bb,
	                                       const sf::FloatRect& region, float& minX, float& maxX, float& minY,
	                                       float& maxY) {
		const float oxl = region.position.x;
		const float oxr = region.position.x + region.size.x;
		const float oyt = region.position.y;
		const float oyb = region.position.y + region.size.y;

		const float dLeft = shapePos.x - bb.position.x;
		const float dRight = bb.position.x + bb.size.x - shapePos.x;
		const float dTop = shapePos.y - bb.position.y;
		const float dBot = bb.position.y + bb.size.y - shapePos.y;

		minX = oxl + dLeft;
		maxX = oxr - dRight;
		minY = oyt + dTop;
		maxY = oyb - dBot;

		if (minX > maxX) {
			const float mid = (minX + maxX) * 0.5f;
			minX = maxX = mid;
		}
		if (minY > maxY) {
			const float mid = (minY + maxY) * 0.5f;
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
	auto vel = (targetPos - platformNode->GetPosGlobal()) * speedFactor;
	vel.x = std::clamp(vel.x, -velLimit.x, velLimit.x);
	vel.y = std::clamp(vel.y, -velLimit.y, velLimit.y);
	platformNode->RequireBehaviour<PhysicsBodyBehaviour>()->SetVelocity(vel);
}

void ClampPongPlatformDesiredCenter(sf::Vector2f& center, bool isBottomPlayer,
                                    const std::shared_ptr<SceneNode>& platformNode,
                                    const std::weak_ptr<SceneNode>& movementBounds) {
	if (!platformNode) {
		return;
	}
	auto* body = platformNode->FindBehaviour<PhysicsBodyBehaviour>().get();
	if (!body) {
		return;
	}
	const sf::FloatRect bb = body->GetBbox();

	if (auto boundsNode = movementBounds.lock()) {
		const sf::FloatRect region = GetRectangleNodeWorldAabb(*boundsNode);
		if (region.size.x > 0.f && region.size.y > 0.f) {
			float minX, maxX, minY, maxY;
			ComputeClampFromAxisAlignedRegion(platformNode->GetPosGlobal(), bb, region, minX, maxX, minY, maxY);
			center.x = std::clamp(center.x, minX, maxX);
			center.y = std::clamp(center.y, minY, maxY);
			return;
		}
	}

	const sf::FloatRect field = GetPongPlayfieldRect();
	const float screenH = GetPongWindowSize().y;

	float minX, maxX, minY, maxY;
	ComputePlatformCenterBounds(platformNode->GetPosGlobal(), bb, isBottomPlayer, field, screenH, minX, maxX, minY,
	                            maxY);

	center.x = std::clamp(center.x, minX, maxX);
	center.y = std::clamp(center.y, minY, maxY);
}

void ClampPongPlatformToPlayfield(const std::shared_ptr<SceneNode>& platformNode, bool isBottomPlayer,
                                  const std::weak_ptr<SceneNode>& movementBounds) {
	if (!platformNode) {
		return;
	}
	sf::Vector2f pos = platformNode->GetPosGlobal();
	ClampPongPlatformDesiredCenter(pos, isBottomPlayer, platformNode, movementBounds);
	platformNode->SetPosGlobal(pos);
}
