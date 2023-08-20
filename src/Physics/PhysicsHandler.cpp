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
		auto pc = body->findComponent<PhysicalComponent>();

		sf::Vector2f forceSum = [&]() {
			sf::Vector2f result;
			if (mGravity != 0.f) {
				result += sf::Vector2f(0.f, mGravity * pc->mMass);
			}
			if (auto pullComp = body->findComponent<UserPullComponent>()) {
				result += pullComp->calcPullForce();
			}
			return result;
		}();

		pc->mVelocity += forceSum / pc->mMass * dt.asSeconds();
		pc->mPos += pc->mVelocity * dt.asSeconds();
		forceSum = sf::Vector2f();
	}

	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			if (auto collisionPoint = findCollisionPoint(it1->lock(), it2->lock())) {
				resolveCollision(it1->lock(), it2->lock(), *collisionPoint);
			}
		}
	}
}

bool PhysicsHandler::checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	return body1->getBbox().intersects(body2->getBbox());
}

std::optional<CollisionDetails> PhysicsHandler::findCollisionPoint(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	if (!checkBboxIntersection(body1, body1)) {
		return std::nullopt;
	}

	auto cmp = [](const sf::Vector2f& a, const sf::Vector2f& b) {
		if (a.x == b.x)
			return a.y < b.y;
		return a.x < b.x;
	};
	std::set<sf::Vector2f, decltype(cmp)> intersectionPoints;

	std::list<Segment> edges1, edges2;
	int pointsCount = body1->getPointCount();
	for (int i = 0; i < pointsCount; ++i) {
		edges1.emplace_back(body1->getPoint(i), body1->getPoint((i + 1) % pointsCount));
	}
	pointsCount = body2->getPointCount();
	for (int i = 0; i < pointsCount; ++i) {
		edges2.emplace_back(body2->getPoint(i), body2->getPoint((i + 1) % pointsCount));
	}

	for (const auto& edge1 : edges1) {
		for (const auto& edge2 : edges2) {
			// todo handle circle bodies in other way
			if (auto intersectionPoint = findSegmentsIntersectionPoint(edge1, edge2)) {
				intersectionPoints.insert(intersectionPoint->p1);
				if (intersectionPoint->p2) {
					intersectionPoints.insert(*intersectionPoint->p2);
				}
			}
		}
	}

	if (intersectionPoints.size() == 0) {
		return std::nullopt;
	}

	CollisionDetails result;
	if (intersectionPoints.size() == 1) {
		result.point = *intersectionPoints.begin();
		auto objToObj = body2->getPhysicalComponent()->mPos - body1->getPhysicalComponent()->mPos;
		result.normalizedTangent = utils::normalize({ -objToObj.y, objToObj.x });
	}
	else {
		auto min = *intersectionPoints.begin();
		auto max = *intersectionPoints.rbegin();
		result.point = (min + max) * 0.5f;
		result.normalizedTangent = utils::normalize(max - min);
	}
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
		if (abs(dxA) > abs(dxB)) {
			intersectionSegment.start.x = std::max(segA.start.x, segB.start.x);
			intersectionSegment.end.x = std::min(segA.end.x, segB.end.x);
			if (intersectionSegment.start.x > intersectionSegment.end.x) {
				return std::nullopt;
			}
			float k = dyA / dxA;
			float b = y1 - x1 * k;
			intersectionSegment.start.y = k * intersectionSegment.start.x + b;
			intersectionSegment.end.y = k * intersectionSegment.end.x + b;
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

void PhysicsHandler::resolveCollision(shared_ptr<AbstractBody>&& body1, shared_ptr<AbstractBody>&& body2, const CollisionDetails& collisionDetails) {
	// todo: consider immovable objects
	// todo remove overlapping
	auto pc1 = body1->requireComponent<PhysicalComponent>();
	auto pc2 = body2->requireComponent<PhysicalComponent>();

	auto v1 = utils::project(pc1->mVelocity, collisionDetails.normalizedTangent);
	auto v2 = utils::project(pc2->mVelocity, collisionDetails.normalizedTangent);

	const auto m1 = pc1->mMass;
	const auto m2 = pc2->mMass;
	auto r = pc1->mRestitution * pc2->mRestitution;

	auto dv1 = -(1.f + r) * m2 / (m1 + m2) * (v1 - v2);
	auto dv2 = (1.f + r) * m1 / (m1 + m2) * (v1 - v2);

	auto normal = sf::Vector2f(-collisionDetails.normalizedTangent.y, collisionDetails.normalizedTangent.x);
	pc1->mVelocity += normal * dv1;
	pc2->mVelocity += normal * dv2;

}
