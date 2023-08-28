#include "PhysicsHandler.h"
#include <cassert>
#include "AbstractBody.h"
#include "CollisionDetails.h"
#include "common.h"
#include "UserPullComponent.h"
#include "Utils.h"

void PhysicsHandler::update(const sf::Time& dt) {
	utils::removeExpiredPointers(mBodies);
	for (int i = 0; i < mSubStepsCount; ++i) {
		updateSubStep(dt / static_cast<float>(mSubStepsCount));
	}
}

void PhysicsHandler::updateSubStep(const sf::Time& dt) {
	for (auto& wBody : mBodies) {
		auto body = wBody.lock();
		auto physComp = body->findComponent<PhysicalComponent>();
		auto pullComp = body->findComponent<UserPullComponent>();

		sf::Vector2f forceSum = [&]() {
			sf::Vector2f result;
			if (mGravity != 0.f) {
				result += sf::Vector2f(0.f, mGravity * physComp->mMass);
			}
			if (pullComp && pullComp->mMode == UserPullComponent::PullMode::SOFT) {
				result += pullComp->getPullVector() * pullComp->mPullingStrength;
			}
			return result;
		}();

		physComp->mVelocity += forceSum / physComp->mMass * dt.asSeconds();

		physComp->mPos += physComp->mVelocity * dt.asSeconds();

		if (pullComp && pullComp->mMode == UserPullComponent::PullMode::HARD) {
			physComp->mPos += pullComp->getPullVector();
			physComp->mVelocity = {};
		}
	}

	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			if (auto collision = detectCollision(it1->lock(), it2->lock())) {
				resolveCollision(*collision);
			}
		}
	}
}


bool PhysicsHandler::checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	return body1->getBbox().intersects(body2->getBbox());
}

std::optional<CollisionDetails> PhysicsHandler::detectCollision(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	if (!checkBboxIntersection(body1, body1)) {
		return std::nullopt;
	}

	std::vector<sf::Vector2f> edges_i_p;
	
	const auto pointsCount1 = body1->getPointCount();
	const auto pointsCount2 = body2->getPointCount();
	CollisionDetails result;

	for (size_t i = 0; i < pointsCount1; ++i) {
		const Segment edge1 = { body1->getPoint(i), body1->getPoint((i + 1) % pointsCount1) };

		for (size_t j = 0; j < pointsCount2; ++j) {
			const Segment edge2 = { body2->getPoint(j), body2->getPoint((j + 1) % pointsCount2) };

			// todo handle circle bodies in other way
			if (auto i_point = findSegmentsIntersectionPoint(edge1, edge2)) {
				if (std::find(edges_i_p.begin(), edges_i_p.end(), i_point->p1) == edges_i_p.end()) {
					edges_i_p.emplace_back(i_point->p1);
				}
				if (i_point->p2) {
					if (std::find(edges_i_p.begin(), edges_i_p.end(), i_point->p2) == edges_i_p.end()) {
						edges_i_p.emplace_back(*i_point->p2);
					}
				}

				result.body1penetratingPoints.insert(i);
				result.body1penetratingPoints.insert((i + 1) % pointsCount1);
				result.body2penetratingPoints.insert(j);
				result.body2penetratingPoints.insert((j + 1) % pointsCount2);
			}
		}
	}

	if (edges_i_p.size() == 0) {
		return std::nullopt;
	}

	if (edges_i_p.size() == 1) {
		result.intersection.start = *edges_i_p.begin(); // I doubt that this is correct way to find start and end
		result.intersection.end = *edges_i_p.begin();
	}
	else {
		result.intersection.start = *edges_i_p.begin();
		result.intersection.end = *edges_i_p.rbegin();
	}

	result.wBody1 = body1;
	result.wBody2 = body2;
	return result;
}

std::optional<SegmentIntersectionPoints> PhysicsHandler::findSegmentsIntersectionPoint(const Segment& segA, const Segment& segB) {
	assert(segA.start != segA.end);
	assert(segB.start != segB.end);

	float x1 = segA.start.x, y1 = segA.start.y;
	float x2 = segA.end.x, y2 = segA.end.y;
	float x3 = segB.start.x, y3 = segB.start.y;
	float x4 = segB.end.x, y4 = segB.end.y;

	float dxA = x2 - x1;
	float dyA = y2 - y1;
	float dxB = x4 - x3;
	float dyB = y4 - y3;

	float den = dxA * dyB - dyA * dxB;

	if (bool isParallel = abs(den) < std::numeric_limits<float>::epsilon()) {
		if (!utils::arePointsCollinear(segA.start, segA.end, segB.start)) {
			return std::nullopt;
		}

		Segment intersectionSegment;
		if (abs(dxA) > abs(dyA)) {
			intersectionSegment.start.x = std::max(segA.start.x, segB.start.x);
			intersectionSegment.end.x = std::min(segA.end.x, segB.end.x);
			if (intersectionSegment.start.x > intersectionSegment.end.x) {
				return std::nullopt;
			}
			float k = dyA / dxA;
			float b = y1 - x1 * k;
			intersectionSegment.start.y = k * intersectionSegment.start.x + b;
			intersectionSegment.end.y = k * intersectionSegment.end.x + b;

			assert(!utils::isNan(intersectionSegment.start));
			assert(!utils::isNan(intersectionSegment.end));
		}
		else {
			intersectionSegment.start.y = std::max(segA.start.y, segB.start.y);
			intersectionSegment.end.y = std::min(segA.end.y, segB.end.y);
			if (intersectionSegment.start.y > intersectionSegment.end.y) {
				return std::nullopt;
			}
			float k = dxA / dyA;
			float b = x1 - y1 * k;
			intersectionSegment.start.x = k * intersectionSegment.start.y + b;
			intersectionSegment.end.x = k * intersectionSegment.end.y + b;

			assert(!utils::isNan(intersectionSegment.start));
			assert(!utils::isNan(intersectionSegment.end));
		}
		return SegmentIntersectionPoints{ intersectionSegment.start, intersectionSegment.end };
	}

	auto dot1 = x3 * y4 - y3 * x4;
	auto dot2 = x1 * y2 - y1 * x2;
	float numX = dxA * dot1 - dxB * dot2;
	float numY = dyA * dot1 - dyB * dot2;
	auto linesIntersection = sf::Vector2f(numX / den, numY / den);

	auto isInsideSegment = [&linesIntersection](const Segment& seg) {
		auto dx = seg.end.x - seg.start.x;
		auto dy = seg.end.y - seg.start.y;
		if (std::abs(dy) > std::abs(dx)) {
			return linesIntersection.y >= std::min(seg.start.y, seg.end.y) && linesIntersection.y <= std::max(seg.start.y, seg.end.y);
		}
		return linesIntersection.x >= std::min(seg.start.x, seg.end.x) && linesIntersection.x <= std::max(seg.start.x, seg.end.x);
	};
	if (!isInsideSegment(segA)) {
		return std::nullopt;
	}
	if (!isInsideSegment(segB)) {
		return std::nullopt;
	}
	return SegmentIntersectionPoints{ linesIntersection };
}

// todo: consider immovable objects
void PhysicsHandler::resolveCollision(const CollisionDetails& collision) {
	auto body1 = collision.wBody1.lock();
	auto body2 = collision.wBody2.lock();
	if (!body1 || !body2) {
		assert(false);
		return;
	}

	//todo make it oriented in half plane
	sf::Vector2f normalizedTangent = [&]() {
		if (collision.intersection.start != collision.intersection.end) {
			return utils::normalize(collision.intersection.start - collision.intersection.end);
		}
		auto objToObj = body2->getPhysicalComponent()->mPos - body1->getPhysicalComponent()->mPos;
		return utils::normalize({ -objToObj.y, objToObj.x });
	}();

	/* velocities handling */
	auto pc1 = body1->requireComponent<PhysicalComponent>();
	auto pc2 = body2->requireComponent<PhysicalComponent>();

	auto v1 = utils::project(pc1->mVelocity, normalizedTangent);
	auto v2 = utils::project(pc2->mVelocity, normalizedTangent);

	const auto m1 = pc1->mMass;
	const auto m2 = pc2->mMass;
	auto r = pc1->mRestitution * pc2->mRestitution;

	auto dv1 = -(1.f + r) * m2 / (m1 + m2) * (v1 - v2);
	auto dv2 = (1.f + r) * m1 / (m1 + m2) * (v1 - v2);

	auto normal = sf::Vector2f(-normalizedTangent.y, normalizedTangent.x);
	pc1->mVelocity += normal * dv1;
	pc2->mVelocity += normal * dv2;

	auto isNan = utils::isNan(pc1->mVelocity) || utils::isNan(pc2->mVelocity);
	assert(!isNan);

	/* penetration fixing */


}