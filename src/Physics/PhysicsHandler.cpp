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
#include "fmt/format.h"

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
		auto pos = body->getPosGlobal();

		sf::Vector2f forceSum = [&]() {
			sf::Vector2f force;
			if (!physComp->isImmovable()) {
				force += mGravity * physComp->mMass;
			}
			if (pullComp && pullComp->mMode == UserPullComponent::PullMode::FORCE) {
				force += pullComp->getPullVector() * pullComp->mPullingStrength;
			}
			return force;
		}();

		physComp->mVelocity += forceSum / physComp->mMass * dt.asSeconds();
		pos += physComp->mVelocity * dt.asSeconds();

		if (pullComp) {
			if (pullComp->mMode == UserPullComponent::PullMode::POSITION) {
				pos += pullComp->getPullVector();
				physComp->mVelocity = {};
			}
			else if (pullComp->mMode == UserPullComponent::PullMode::VELOCITY) {
				physComp->mVelocity = pullComp->getPullVector();
			}
		}
		body->setPosGlobal(pos);
	}

	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			if (auto collision = detectCollision(it1->lock(), it2->lock())) {
				resolveCollision(*collision);
			}
		}
	}

	if (EI()->isDebugDrawEnabled()) {
		float systemEnergy = 0.f;
		sf::Vector2f systemImpulse;
		for (auto& wBody : mBodies) {
			if (auto body = wBody.lock()) {
				if (body->getPhysicalComponent()->isImmovable()) {
					continue;
				}
				auto v = body->getPhysicalComponent()->mVelocity;
				auto v_s = utils::length(v);
				auto m = body->getPhysicalComponent()->mMass;
				systemImpulse += v * m;
				systemEnergy += m * v_s * v_s;
			}
		}
		fmt::println("E = {}, P = {}", systemEnergy, utils::toString(systemImpulse));
	}
}


bool PhysicsHandler::checkBboxIntersection(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	return body1->getBbox().intersects(body2->getBbox());
}

std::optional<CollisionDetails> PhysicsHandler::detectCollision(const shared_ptr<AbstractBody>& body1, const shared_ptr<AbstractBody>& body2) {
	if (!body1 || !body2) {
		assert(false);
		return std::nullopt;
	}

	if (body1->getPhysicalComponent()->isImmovable() && body2->getPhysicalComponent()->isImmovable()) {
		return std::nullopt;
	}

	if (!checkBboxIntersection(body1, body1)) {
		return std::nullopt;
	}

	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);
	
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
void PhysicsHandler::resolveCollision(const CollisionDetails& collision) {
	auto body1 = collision.wBody1.lock();
	auto body2 = collision.wBody2.lock();
	if (!body1 || !body2) {
		assert(false);
		return;
	}

	auto pc1 = body1->findComponent<PhysicalComponent>();
	auto pc2 = body2->findComponent<PhysicalComponent>();
	
	if (pc1->isImmovable()) { // fixed body must be second
		std::swap(body1, body2);
		std::swap(pc1, pc2);
	}

	const auto m1 = pc1->mMass;
	const auto m2 = pc2->mMass;
	auto v1_to_v2 = pc2->mVelocity - pc1->mVelocity;
	auto b1_tangent = collision.intersection.getDirVector();
	auto b1_normal = utils::normalize(sf::Vector2f(-b1_tangent.y, b1_tangent.x));
	if (utils::dot(body2->getPosGlobal() - body1->getPosGlobal(), b1_normal) < 0.f) {
		b1_tangent = -b1_tangent;
		b1_normal = -b1_normal;
	}

	/* overlapping fixing */
	auto getPenetrationDepth = [collisionPoint = collision.intersection.start](const AbstractBody* body, const sf::Vector2f& bodyNormal) {
		float result = 0.f;
		for (size_t i = 0; i < body->getPointCount(); ++i) {
			auto point = body->getPointGlobal(i);
			auto penetrationVec = body->getPointGlobal(i) - collisionPoint;
			float depth = utils::project(penetrationVec, bodyNormal);
			result = std::max(result, depth);
		}
		return result;
	};
	float penetrationDepthSum = getPenetrationDepth(body1.get(), b1_normal) + getPenetrationDepth(body2.get(), -b1_normal);

	if (pc2->isImmovable()) {
		auto b1_pos = body1->getPosGlobal() - b1_normal * penetrationDepthSum;
		body1->setPosGlobal(b1_pos);
	}
	else {
		auto b1_pos = body1->getPosGlobal() - b1_normal * penetrationDepthSum * (m1 / (m1 + m2));
		body1->setPosGlobal(b1_pos);
		auto b2_pos = body2->getPosGlobal() + b1_normal * penetrationDepthSum * (m2 / (m1 + m2));
		body2->setPosGlobal(b2_pos);
	}

	/* velocities handling */
	if (bool areMovingTowards = utils::dot(b1_normal, v1_to_v2) < 0.f) {
		const auto r = pc1->mRestitution * pc2->mRestitution;

		auto norm_v1 = utils::project(pc1->mVelocity, b1_normal);
		auto norm_v2 = utils::project(pc2->mVelocity, b1_normal);
		auto norm_v_diff = norm_v1 - norm_v2;

		float norm_dv1 = 0.f;
		float norm_dv2 = 0.f;

		if (pc2->isImmovable()) {
			norm_dv1 = -(1.f + r) * norm_v_diff;
		}
		else {
			norm_dv1 = -(1.f + r) * m2 / (m1 + m2) * norm_v_diff;
			norm_dv2 = (1.f + r) * m1 / (m1 + m2) * norm_v_diff;
		}

		auto dv1 = b1_normal * norm_dv1;
		auto dv2 = b1_normal * norm_dv2;
		pc1->mVelocity += dv1;
		pc2->mVelocity += dv2;

		auto isNan = utils::isNan(pc1->mVelocity) || utils::isNan(pc2->mVelocity);
		assert(!isNan);

		if (EI()->isDebugDrawEnabled()) {
			auto window = EI()->getMainWindow();
			{
				VectorArrow force1(body1->getPosGlobal(), body1->getPosGlobal() + dv1);
				if (auto shapedBody = dynamic_cast<AbstractShapeBody*>(body1.get())) {
					auto color = shapedBody->getBaseShape()->getFillColor();
					color.a = 255u;
					force1.setColor(color);
				}
				window->draw(force1);
			}
			{
				VectorArrow force2(body2->getPosGlobal(), body2->getPosGlobal() + dv2);
				if (auto shapedBody = dynamic_cast<AbstractShapeBody*>(body2.get())) {
					auto color = shapedBody->getBaseShape()->getFillColor();
					color.a = 255u;
					force2.setColor(color);
				}
				window->draw(force2);
			}
		}
		
	}

	if (EI()->isDebugDrawEnabled()) {
		auto window = EI()->getMainWindow();
		const sf::Vector2f middlePoint((collision.intersection.start + collision.intersection.end) * 0.5f);

		{	// collision segment
			VectorArrow segment(collision.intersection.start, collision.intersection.end, sf::Color::Magenta);
			window->draw(segment);
		}

		{	// normal
			VectorArrow tangentArrow(middlePoint, middlePoint + b1_normal * 50.f, sf::Color::Yellow);
			window->draw(tangentArrow);
		}

		{	// collision center
			const float radius = 2.f;
			sf::CircleShape middlePointCircle(radius, 10);
			middlePointCircle.setPosition(middlePoint - sf::Vector2f(radius, radius));
			middlePointCircle.setFillColor(sf::Color::White);
			window->draw(middlePointCircle);
		}
	}
}