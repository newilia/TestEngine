#include "PhysicsHandler.h"

#include <cassert>

#include "AbstractBody.h"
#include "common.h"
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
		if (mGravity != 0.f) {
			pc->mForce += sf::Vector2f(0.f, mGravity * pc->mMass);
		}
		pc->mVelocity += pc->mForce / pc->mMass * dt.asSeconds();
		pc->mPos += pc->mVelocity * dt.asSeconds();
		pc->mForce = sf::Vector2f();
	}

	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			if (auto collisionPoint = findCollisionPoint(it1->lock(), it2->lock())) {
				resolveCollision(it1->lock(), it2->lock(), *collisionPoint);
			}
		}
	}
}

bool PhysicsHandler::checkBboxIntersection(const shared_ptr<AbstractBody>& obj1, const shared_ptr<AbstractBody>& obj2) {
	return obj1->getBbox().intersects(obj2->getBbox());
}

std::optional<sf::Vector2f> PhysicsHandler::findCollisionPoint(const shared_ptr<AbstractBody>& obj1, const shared_ptr<AbstractBody>& obj2) {
	if (!checkBboxIntersection(obj1, obj1)) {
		return std::nullopt;
	}

	sf::Vector2f collisionPoint;
	int intersectingEdgesCount = 0;

	int pointsCount1 = obj1->getPointCount();
	for (int i = 0; i < pointsCount1 - 1; ++i) {
		Segment obj1Edge(obj1->getPoint(i), obj1->getPoint(i + 1));

		int pointsCount2 = obj2->getPointCount();
		for (int j = 0; j < pointsCount2 - 1; ++j) {
			Segment obj2Edge(obj2->getPoint(j), obj2->getPoint(j + 1));

			// todo handle circle bodies in other way
			if (auto intersectionPoint = findSegmentsIntersectionPoint(obj1Edge, obj2Edge)) {
				collisionPoint += *intersectionPoint;
				++intersectingEdgesCount;
			}
		}
	}

	if (intersectingEdgesCount == 0) {
		return std::nullopt;
	}
	collisionPoint /= static_cast<float>(intersectingEdgesCount);
	return collisionPoint;
}

std::optional<sf::Vector2f> PhysicsHandler::findSegmentsIntersectionPoint(const Segment& segA, const Segment& segB) {
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
		return (intersectionSegment.start + intersectionSegment.end) / 2.f;
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
	return linesIntersection;
}

void PhysicsHandler::resolveCollision(shared_ptr<AbstractBody>&& body1, shared_ptr<AbstractBody>&& body2, sf::Vector2f collisionPoint) {
	// todo: consider bounce
	auto pc1 = body1->requireComponent<PhysicalComponent>();
	auto pc2 = body2->requireComponent<PhysicalComponent>();

	auto collisionNormal = pc2->mPos - pc1->mPos;
	float massSum = pc1->mMass + pc2->mMass;
	auto reflectedSpeed1 = utils::reflect(pc1->mVelocity, collisionNormal) * (pc1->mMass / massSum);
	auto reflectedSpeed2 = utils::reflect(pc2->mVelocity, collisionNormal) * (pc2->mMass / massSum);

	auto totalImpulse = pc1->mMass * pc1->mVelocity + pc2->mMass * pc2->mVelocity;
	float body1massPart = pc1->mMass / massSum;
	float body2massPart = pc2->mMass / massSum;

	float bounce = (pc1->mBounce + pc2->mBounce) / 2.f;/*
	pc1->mVelocity = pc1->mVelocity * body1massPart + reflectedSpeed1 * body2massPart;
	pc2->mVelocity = pc2->mVelocity * body2massPart + reflectedSpeed2 * body1massPart;*/
	pc1->mVelocity = totalImpulse / massSum;
	pc2->mVelocity = totalImpulse / massSum;
}
