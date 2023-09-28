#include "PhysicsHandler.h"
#include <cassert>

#include "AbstractBody.h"
#include "IntersectionDetails.h"
#include "common.h"
#include "EngineInterface.h"
#include "AbstractShapeBody.h"
#include "ShapeBody.h"
#include "UserPullComponent.h"
#include "Utils.h"
#include "VectorArrow.h"
#include "fmt/format.h"
#include <optional>

#include "CollisionComponent.h"
#include "OverlappingComponent.h"

void PhysicsHandler::update(const sf::Time& dt) {
	utils::removeExpiredPointers(mBodies);
	for (int i = 0; i < mSubStepsCount; ++i) {
		updateSubStep(dt / static_cast<float>(mSubStepsCount));
	}
}

void PhysicsHandler::registerBody(shared_ptr<AbstractBody> body) {
	mBodies.emplace_back(body);
}

void PhysicsHandler::unregisterBody(AbstractBody* body) {
	/*auto it = std::find(mBodies.begin(), mBodies.end(), body);
	if (it != mBodies.end()) {
		mBodies.erase(it);
	}*/
}

void PhysicsHandler::updateSubStep(const sf::Time& dt) {
	// motion step
	for (auto& wBody : mBodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto physComp = body->findComponent<PhysicalComponent>();
		auto pullComp = body->findComponent<UserPullComponent>();
		auto pos = body->getPosGlobal();

		sf::Vector2f forceSum = [&]() {
			sf::Vector2f force;
			if (pullComp && pullComp->mMode == UserPullComponent::PullMode::FORCE) {
				force += pullComp->getPullVector() * pullComp->mPullingStrength;
			}
			return force;
		}();

		physComp->mVelocity += forceSum / physComp->mMass * dt.asSeconds();
		if (mIsGravityEnabled && !physComp->isImmovable()) {
			physComp->mVelocity += mGravity * dt.asSeconds();
		}

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

	// handle intersections
	for (auto it1 = mBodies.begin(); it1 != mBodies.end(); ++it1) {
		auto b1 = it1->lock();
		if (!b1) {
			continue;
		}

		for (auto it2 = std::next(it1); it2 != mBodies.end(); ++it2) {
			auto b2 = it2->lock();
			if (!b2) {
				continue;
			}

			if (auto intersection = detectIntersection(b1.get(), b2.get())) {
				{
					auto cc1 = b1->findComponent<CollisionComponent>();
					auto cc2 = b2->findComponent<CollisionComponent>();
					if (cc1 && cc2 && (cc1->mCollisionGroups & cc2->mCollisionGroups).any()) {
						if (!b1->getPhysicalComponent()->isImmovable() || !b2->getPhysicalComponent()->isImmovable()) {
							resolveCollision(*intersection);
							for (auto callback : cc1->mCollisionCallbacks) {
								callback->operator()(*intersection);
							}
							for (auto callback : cc2->mCollisionCallbacks) {
								callback->operator()(*intersection);
							}
						}
					}
				}

				{
					auto oc1 = b1->findComponent<OverlappingComponent>();
					auto oc2 = b2->findComponent<OverlappingComponent>();
					if (oc1 && oc2 && (oc1->mOverlappingGroups & oc2->mOverlappingGroups).any()) {
						for (auto callback : oc1->mOverlappingCallbacks) {
							callback->operator()(*intersection);
						}
					}
				}
			}
		}
	}

	if (EI()->isDebugEnabled()) {
		//float systemEnergy = 0.f;
		//sf::Vector2f systemImpulse;
		//for (auto& wBody : mBodies) {
		//	if (auto body = wBody.lock()) {
		//		if (body->getPhysicalComponent()->isImmovable()) {
		//			continue;
		//		}
		//		auto v = body->getPhysicalComponent()->mVelocity;
		//		auto v_s = utils::length(v);
		//		auto m = body->getPhysicalComponent()->mMass;
		//		systemImpulse += v * m;
		//		systemEnergy += m * v_s * v_s;
		//	}
		//}
		//fmt::println("E = {}, P = {}", systemEnergy, utils::toString(systemImpulse));
	}
}


bool PhysicsHandler::checkBboxIntersection(const AbstractBody* body1, const AbstractBody* body2) {
	auto&& bb1 = body1->getBbox();
	auto&& bb2 = body2->getBbox();
	return bb1.intersects(bb2);
}

std::optional<IntersectionDetails> PhysicsHandler::detectIntersection(const AbstractBody* body1, const AbstractBody* body2) {
	if (!body1 || !body2) {
		assert(false);
		return std::nullopt;
	}

	if (!checkBboxIntersection(body1, body2)) {
		return std::nullopt;
	}

	if (auto circle1 = dynamic_cast<const CircleBody*>(body1)) {
		if (auto circle2 = dynamic_cast<const CircleBody*>(body2)) {
			return detectCircleCircleIntersection(circle1, circle2);
		}
		return detectCirclePolygonIntersection(circle1, body2);
	}
	if (auto circle2 = dynamic_cast<const CircleBody*>(body2)) {
		return detectCirclePolygonIntersection(circle2, body1);
	}
	return detectPolygonPolygonIntersection(body1, body2);
}

std::optional<IntersectionDetails> PhysicsHandler::detectPolygonPolygonIntersection(const AbstractBody* body1, const AbstractBody* body2) {

	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);

	const auto pointsCount1 = body1->getPointCount();
	const auto pointsCount2 = body2->getPointCount();
	IntersectionDetails result;

	for (size_t i = 0; i < pointsCount1; ++i) {
		const Segment edge1 = { body1->getPointGlobal(i), body1->getPointGlobal((i + 1) % pointsCount1) };

		for (size_t j = 0; j < pointsCount2; ++j) {
			const Segment edge2 = { body2->getPointGlobal(j), body2->getPointGlobal((j + 1) % pointsCount2) };
			
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

	result.wBody1 = utils::sharedPtrCast<AbstractBody>(body1);
	result.wBody2 = utils::sharedPtrCast<AbstractBody>(body2);
	return result;
}

std::optional<IntersectionDetails> PhysicsHandler::detectCirclePolygonIntersection(const CircleBody* circle, const AbstractBody* body) {

	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);
	
	const auto pointsCount = body->getPointCount();
	IntersectionDetails result;

	for (size_t i = 0; i < pointsCount; ++i) {
		const Segment edge = { body->getPointGlobal(i), body->getPointGlobal((i + 1) % pointsCount) };
		
		if (auto i_point = findSegmentCircleIntersectionPoint(edge, circle->getShape()->getPosition(), circle->getShape()->getRadius())) {
			edges_i_p.emplace_back(i_point->p1);
			if (i_point->p2) {
				edges_i_p.emplace_back(*i_point->p2);
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

	result.wBody1 = utils::sharedPtrCast<AbstractBody>(circle);
	result.wBody2 = utils::sharedPtrCast<AbstractBody>(body);
	return result;
}

std::optional<IntersectionDetails> PhysicsHandler::detectCircleCircleIntersection(const CircleBody* circle1, const CircleBody* circle2) {
	using namespace utils;
	auto r1 = circle1->getShape()->getRadius();
	auto r2 = circle2->getShape()->getRadius();
	auto pos1 = circle1->getShape()->getPosition();
	auto pos2 = circle2->getShape()->getPosition() - circle1->getShape()->getPosition();
	float a = -2 * pos2.x;
	float b = -2 * pos2.y;
	float c = sq(pos2.x) + sq(pos2.y) + sq(r1) - sq(r2);
	auto p = sq(c) - sq(r1) * (sq(a) + sq(b));
	if (p > std::numeric_limits<float>::epsilon()) {
		return std::nullopt;
	}
	float x0 = -a * c / (sq(a) + sq(b)) + pos1.x;
	float y0 = -b * c / (sq(a) + sq(b)) + pos1.y;

	IntersectionDetails result;
	result.wBody1 = utils::sharedPtrCast<AbstractBody>(circle1);
	result.wBody2 = utils::sharedPtrCast<AbstractBody>(circle2);

	if (isZero(p)) {
		result.intersection.start = sf::Vector2f(x0, y0);
		result.intersection.end = result.intersection.start;
	}
	else {
		float d = sq(r1) - sq(c) / (sq(a) + sq(b));
		float mult = sqrt(d / (sq(a) + sq(b)));
		result.intersection.start.x = x0 + b * mult;
		result.intersection.start.y = y0 - a * mult;
		result.intersection.end.x = x0 - b * mult;
		result.intersection.end.y = y0 + a * mult;
	}
	if (EI()->isDebugEnabled()) {
		auto wnd = EI()->getMainWindow();
		wnd->draw(createCircle(result.intersection.start, 2, sf::Color::White));
		wnd->draw(createCircle(result.intersection.end, 2, sf::Color::White));
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

std::optional<SegmentIntersectionPoints> PhysicsHandler::findSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter, float radius) {
	using namespace utils;
	SegmentIntersectionPoints result;
	auto s = seg;
	s.start -= circleCenter;
	s.end -= circleCenter;

	auto v = seg.getDirVector();
	bool is_xy_swapped = false;
	if (isZero(v.x)) {
		is_xy_swapped = true;
		std::swap(v.x, v.y);
		std::swap(s.start.x, s.start.y);
		std::swap(s.end.x, s.end.y);
	}

	if (s.start.x > s.end.x) {
		std::swap(s.start, s.end);
	}

	float slope = v.y / v.x;
	float a = sq(slope) + 1;
	float b = slope * s.start.y - sq(slope);
	float c = sq(s.start.y) + sq(slope) * s.start.x - s.start.x * s.start.y * slope - sq(radius);

	auto x_resolution = solveQuadraticEquation(a, b, c);
	if (!x_resolution) {
		return std::nullopt;
	}

	bool isFirstPointInSegment = x_resolution->first >= s.start.x && x_resolution->first <= s.end.x;
	if (isFirstPointInSegment) {
		result.p1.x = x_resolution->first;
		result.p1.y = s.start.y + (x_resolution->first - s.start.x) * slope;
		if (is_xy_swapped) {
			std::swap(result.p1.x, result.p1.y);
		}
	}

	bool isSecondPointInSegment = false;
	if (x_resolution->second) {
		if (*x_resolution->second >= s.start.x && *x_resolution->second <= s.end.x) {
			isSecondPointInSegment = true;
			sf::Vector2f* p = &result.p1;
			if (isFirstPointInSegment) {
				result.p2 = sf::Vector2f();
				p = &result.p2.value();
			}
			p->x = *x_resolution->second;
			p->y = s.start.y + (*x_resolution->second - s.start.x) * slope;
			if (is_xy_swapped) {
				std::swap(p->x, p->y);
			}
		}
	}
	if (!isFirstPointInSegment && !isSecondPointInSegment) {
		return std::nullopt;
	}

	result.p1 += circleCenter;
	if (result.p2) {
		*result.p2 += circleCenter;
	}

	if (EI()->isDebugEnabled()) {
		auto window = EI()->getMainWindow();
		window->draw(createCircle(result.p1, 2.f, sf::Color::White));
		if (result.p2) {
			window->draw(createCircle(*result.p2, 2.f, sf::Color::White));
		}
	}
	return result;
}

void PhysicsHandler::resolveCollision(const IntersectionDetails& collision) {
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

		if (EI()->isDebugEnabled()) {
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

	if (EI()->isDebugEnabled()) {
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

		window->draw(utils::createCircle(middlePoint, 2, sf::Color::White));
	}
}