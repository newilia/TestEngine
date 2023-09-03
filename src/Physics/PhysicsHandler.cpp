#include "PhysicsHandler.h"
#include <cassert>
#include <iostream>

#include "AbstractBody.h"
#include "CollisionDetails.h"
#include "common.h"
#include "EngineInterface.h"
#include "AbstractShapeBody.h"
#include "UserPullComponent.h"
#include "Utils.h"
#include "VectorArrow.h"

#define PHYSICS_DEBUG 1

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
			sf::Vector2f force;
			if (mGravity != 0.f) {
				force += sf::Vector2f(0.f, mGravity * physComp->mMass);
			}
			if (pullComp && pullComp->mMode == UserPullComponent::PullMode::FORCE) {
				force += pullComp->getPullVector() * pullComp->mPullingStrength;
			}
			return force;
		}();

		physComp->mVelocity += forceSum / physComp->mMass * dt.asSeconds();
		physComp->mPos += physComp->mVelocity * dt.asSeconds();

		if (pullComp) {
			if (pullComp->mMode == UserPullComponent::PullMode::POSITION) {
				physComp->mPos += pullComp->getPullVector();
				physComp->mVelocity = {};
			}
			else if (pullComp->mMode == UserPullComponent::PullMode::VELOCITY) {
				physComp->mVelocity = pullComp->getPullVector();
			}
		}
		
	}

	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			if (auto collision = detectCollision(it1->lock(), it2->lock())) {
				resolveCollision(*collision, dt);
			}
		}
	}

#if PHYSICS_DEBUG
	float systemEnergy = 0.f;
	sf::Vector2f systemImpulse;
	for (auto& wBody : mBodies) {
		if (auto body = wBody.lock()) {
			auto v = body->getPhysicalComponent()->mVelocity;
			auto v_s = utils::length(v);
			auto m = body->getPhysicalComponent()->mMass;
			systemImpulse += v * m;
			systemEnergy += m * v_s * v_s;
		}
	}
	std::cout << "P = " << utils::toString(systemImpulse) << "; E = " << systemEnergy << std::endl;
#endif
}


bool PhysicsHandler::checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	return body1->getBbox().intersects(body2->getBbox());
}

std::optional<CollisionDetails> PhysicsHandler::detectCollision(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	if (!body1 || !body2) {
		assert(false);
		return std::nullopt;
	}

	if (!checkBboxIntersection(body1, body1)) {
		return std::nullopt;
	}

	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(4u);
	
	const auto pointsCount1 = body1->getPointCount();
	const auto pointsCount2 = body2->getPointCount();
	CollisionDetails result;

	for (size_t i = 0; i < pointsCount1; ++i) {
		const Segment edge1 = { body1->getPointGlobal(i), body1->getPointGlobal((i + 1) % pointsCount1) };

		for (size_t j = 0; j < pointsCount2; ++j) {
			const Segment edge2 = { body2->getPointGlobal(j), body2->getPointGlobal((j + 1) % pointsCount2) };

			// todo handle circle bodies in other way
			if (auto i_point = findSegmentsIntersectionPoint(edge1, edge2)) {
				edges_i_p.emplace_back(i_point->p1);
				if (i_point->p2) {
					edges_i_p.emplace_back(*i_point->p2);
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
		result.intersection.start = *edges_i_p.begin();
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
void PhysicsHandler::resolveCollision(const CollisionDetails& collision, const sf::Time& dt) {
	auto body1 = collision.wBody1.lock();
	auto body2 = collision.wBody2.lock();
	if (!body1 || !body2) {
		assert(false);
		return;
	}

	auto pc1 = body1->requireComponent<PhysicalComponent>();
	auto pc2 = body2->requireComponent<PhysicalComponent>();
	auto v1_to_v2 = pc2->mVelocity - pc1->mVelocity;
	auto pos1_to_pos2 = pc2->mPos - pc1->mPos; // assume this as collision normal
	bool areMovingTowards = utils::dot(pos1_to_pos2, v1_to_v2) < 0.f;

	/* TODO penetration fixing */ 
	/*if (areMovingTowards) {
		pc1->mPos -= pc1->mVelocity * dt.asSeconds();
		pc2->mPos -= pc2->mVelocity * dt.asSeconds();
	}*/

	/* velocities handling */
	const sf::Vector2f tangent = [&]() {
		if (collision.intersection.start == collision.intersection.end) {
			return sf::Vector2f(-pos1_to_pos2.y, pos1_to_pos2.x);
		}
		return collision.intersection.end - collision.intersection.start;
	}();

	auto normal = utils::normalize({ -tangent.y, tangent.x });
	const auto m1 = pc1->mMass;
	const auto m2 = pc2->mMass;

	auto norm_v1 = utils::project(pc1->mVelocity, normal);
	auto norm_v2 = utils::project(pc2->mVelocity, normal);
	auto norm_v_diff = norm_v1 - norm_v2;

	auto r = pc1->mRestitution * pc2->mRestitution;

	auto norm_dv1 = -(1.f + r) * m2 / (m1 + m2) * norm_v_diff;
	auto norm_dv2 = (1.f + r) * m1 / (m1 + m2) * norm_v_diff;
	auto dv1 = normal * norm_dv1;
	auto dv2 = normal * norm_dv2;

	if (areMovingTowards) { // objects are moving towards each other
		pc1->mVelocity += dv1;
		pc2->mVelocity += dv2;

		auto isNan = utils::isNan(pc1->mVelocity) || utils::isNan(pc2->mVelocity);
		assert(!isNan);
	}
	
#if PHYSICS_DEBUG
	auto window = EI()->getMainWindow();
	const sf::Vector2f middlePoint((collision.intersection.start + collision.intersection.end) * 0.5f);

	{	// collision segment
		VectorArrow segment(collision.intersection.start, collision.intersection.end, sf::Color::White);
		window->draw(segment);
	}

	{	// normal
		VectorArrow tangentArrow(middlePoint, middlePoint + normal * 50.f, sf::Color::Yellow);
		window->draw(tangentArrow);
	}

	{	// collision center
		const float radius = 2.f;
		sf::CircleShape middlePointCircle(radius, 10);
		middlePointCircle.setPosition(middlePoint - sf::Vector2f(radius, radius));
		middlePointCircle.setFillColor(sf::Color::White);
		window->draw(middlePointCircle);
	}
	
	{
		VectorArrow force1(body1->getGlobalCenter(), body1->getGlobalCenter() + dv1);
		if (auto shapedBody = dynamic_cast<AbstractShapeBody*>(body1.get())) {
			auto color = shapedBody->getBaseShape()->getFillColor();
			color.a = 255u;
			force1.setColor(color);
		}
		window->draw(force1);
	}
	{
		VectorArrow force2(body2->getGlobalCenter(), body2->getGlobalCenter() + dv2);
		if (auto shapedBody = dynamic_cast<AbstractShapeBody*>(body2.get())) {
			auto color = shapedBody->getBaseShape()->getFillColor();
			color.a = 255u;
			force2.setColor(color);
		}
		window->draw(force2);
	}
#endif

}