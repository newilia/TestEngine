#include "PhysicsProcessor.h"

#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/AbstractBody.h"
#include "Engine/Behaviour/Physics/CollisionBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/OverlappingBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/VectorArrowVisual.h"
#include "fmt/format.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <algorithm>
#include <cassert>
#include <deque>
#include <optional>
#include <vector>

void PhysicsProcessor::Update(const sf::Time& dt) {
	Utils::RemoveExpiredPointers(_bodies);
	// motion step
	for (auto& wBody : _bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto rigidBody = body->RequireBehaviour<RigidBodyBehaviour>();
		auto pos = body->GetPosGlobal();

		if (_isGravityEnabled && !rigidBody->IsImmovable()) {
			rigidBody->_velocity += _gravity * dt.asSeconds();
		}

		pos += rigidBody->_velocity * dt.asSeconds();
		body->SetPosGlobal(pos);
	}

	// handle intersections — sweep-and-prune broad phase, then masks + narrow phase
	struct BodySweepEntry
	{
		std::shared_ptr<SceneNode> node;
		ShapeColliderBehaviourBase* collider = nullptr;
		float minX = 0.f;
		float maxX = 0.f;
		/// Same ordering as iteration over `_bodies` in the old all-pairs loop (outer = first).
		size_t listOrder = 0;
	};

	auto bboxXExtents = [](const sf::FloatRect& bb) {
		float x0 = bb.position.x;
		float x1 = bb.position.x + bb.size.x;
		if (x0 > x1) {
			std::swap(x0, x1);
		}
		return std::pair{x0, x1};
	};
	auto pairNeedsNarrowPhase = [](const SceneNode& n1, const SceneNode& n2) {
		auto c1 = n1.FindBehaviour<CollisionBehaviour>();
		auto c2 = n2.FindBehaviour<CollisionBehaviour>();
		auto o1 = n1.FindBehaviour<OverlappingBehaviour>();
		auto o2 = n2.FindBehaviour<OverlappingBehaviour>();
		const bool collisionPair = c1 && c2 && (c1->_collisionGroups & c2->_collisionGroups).any();
		const bool overlapPair = o1 && o2 && (o1->_overlappingGroups & o2->_overlappingGroups).any();
		return collisionPair || overlapPair;
	};

	std::vector<BodySweepEntry> sweepEntries;
	sweepEntries.reserve(_bodies.size());
	size_t bodyListIndex = 0;
	for (auto& wBody : _bodies) {
		auto node = wBody.lock();
		if (!node) {
			++bodyListIndex;
			continue;
		}
		auto* collider = node->FindShapeCollider();
		if (!collider) {
			++bodyListIndex;
			continue;
		}
		auto [minX, maxX] = bboxXExtents(collider->GetBbox());
		sweepEntries.push_back({std::move(node), collider, minX, maxX, bodyListIndex});
		++bodyListIndex;
	}

	std::sort(sweepEntries.begin(), sweepEntries.end(),
	          [](const BodySweepEntry& a, const BodySweepEntry& b) { return a.minX < b.minX; });

	std::deque<size_t> active;
	for (size_t i = 0; i < sweepEntries.size(); ++i) {
		while (!active.empty() && sweepEntries[active.front()].maxX < sweepEntries[i].minX) {
			active.pop_front();
		}
		for (size_t aj : active) {
			auto& eA = sweepEntries[aj];
			auto& eB = sweepEntries[i];
			if (!CheckBboxIntersection(eA.collider, eB.collider)) {
				continue;
			}
			if (!pairNeedsNarrowPhase(*eA.node, *eB.node)) {
				continue;
			}
			const bool aIsFirstInBodyList = eA.listOrder < eB.listOrder;
			auto& nFirst = aIsFirstInBodyList ? eA.node : eB.node;
			auto& nSecond = aIsFirstInBodyList ? eB.node : eA.node;
			auto* cFirst = aIsFirstInBodyList ? eA.collider : eB.collider;
			auto* cSecond = aIsFirstInBodyList ? eB.collider : eA.collider;
			if (auto intersection = DetectIntersection(nFirst, nSecond, cFirst, cSecond, true)) {
				auto b1Collision = nFirst->FindBehaviour<CollisionBehaviour>();
				auto b2Collision = nSecond->FindBehaviour<CollisionBehaviour>();
				auto b1RigidBody = nFirst->RequireBehaviour<RigidBodyBehaviour>();
				auto b2RigidBody = nSecond->RequireBehaviour<RigidBodyBehaviour>();
				if (b1Collision && b2Collision && b1RigidBody && b2RigidBody &&
				    (b1Collision->_collisionGroups & b2Collision->_collisionGroups).any()) {
					if (!b1RigidBody->IsImmovable() || !b2RigidBody->IsImmovable()) {
						ResolveCollision(*intersection);
						b1Collision->_collisionCallbacks.Emit(*intersection);
						b2Collision->_collisionCallbacks.Emit(*intersection);
					}
				}

				auto b1Overlapping = nFirst->FindBehaviour<OverlappingBehaviour>();
				auto b2Overlapping = nSecond->FindBehaviour<OverlappingBehaviour>();
				if (b1Overlapping && b2Overlapping &&
				    (b1Overlapping->_overlappingGroups & b2Overlapping->_overlappingGroups).any()) {
					b1Overlapping->_overlappingCallbacks.Emit(*intersection);
				}
			}
		}
		active.push_back(i);
	}
}

void PhysicsProcessor::RegisterBody(shared_ptr<SceneNode> body) {
	_bodies.emplace_back(body);
}

void PhysicsProcessor::UnregisterBody(SceneNode* body) {
	_bodies.remove_if([body](const std::weak_ptr<SceneNode>& w) {
		auto locked = w.lock();
		return !locked || locked.get() == body;
	});
}

bool PhysicsProcessor::CheckBboxIntersection(const AbstractBody* body1, const AbstractBody* body2) {
	auto&& bb1 = body1->GetBbox();
	auto&& bb2 = body2->GetBbox();
	return bb1.findIntersection(bb2).has_value();
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectIntersection(const shared_ptr<SceneNode>& n1,
                                                                        const shared_ptr<SceneNode>& n2,
                                                                        AbstractBody* c1, AbstractBody* c2,
                                                                        bool bboxAlreadyVerified) {
	if (!n1 || !n2) {
		assert(false);
		return std::nullopt;
	}
	if (!c1 || !c2) {
		return std::nullopt;
	}

	if (!bboxAlreadyVerified && !CheckBboxIntersection(c1, c2)) {
		return std::nullopt;
	}

	auto assignNodes = [&](IntersectionDetails& r) {
		r.wNode1 = n1;
		r.wNode2 = n2;
	};

	auto* col1 = static_cast<ShapeColliderBehaviourBase*>(c1);
	auto* col2 = static_cast<ShapeColliderBehaviourBase*>(c2);

	if (auto* circ1 = dynamic_cast<sf::CircleShape*>(col1->GetBaseShape())) {
		if (auto* circ2 = dynamic_cast<sf::CircleShape*>(col2->GetBaseShape())) {
			if (auto r = DetectCircleCircleIntersection(circ1, circ2)) {
				assignNodes(*r);
				return r;
			}
			return std::nullopt;
		}
		if (auto r = DetectCirclePolygonIntersection(circ1, col2)) {
			assignNodes(*r);
			return r;
		}
		return std::nullopt;
	}
	if (auto* circ2 = dynamic_cast<sf::CircleShape*>(col2->GetBaseShape())) {
		if (auto r = DetectCirclePolygonIntersection(circ2, col1)) {
			assignNodes(*r);
			return r;
		}
		return std::nullopt;
	}
	if (auto r = DetectPolygonPolygonIntersection(col1, col2)) {
		assignNodes(*r);
		return r;
	}
	return std::nullopt;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectPolygonPolygonIntersection(const AbstractBody* body1,
                                                                                      const AbstractBody* body2) {
	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);

	const auto pointsCount1 = body1->GetPointCount();
	const auto pointsCount2 = body2->GetPointCount();
	IntersectionDetails result;

	for (size_t i = 0; i < pointsCount1; ++i) {
		const Segment edge1 = {body1->GetPointGlobal(i), body1->GetPointGlobal((i + 1) % pointsCount1)};

		for (size_t j = 0; j < pointsCount2; ++j) {
			const Segment edge2 = {body2->GetPointGlobal(j), body2->GetPointGlobal((j + 1) % pointsCount2)};

			if (auto i_point = FindSegmentsIntersectionPoint(edge1, edge2)) {
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

	return result;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectCirclePolygonIntersection(const sf::CircleShape* circle,
                                                                                     const AbstractBody* body) {
	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);

	const auto pointsCount = body->GetPointCount();
	IntersectionDetails result;

	for (size_t i = 0; i < pointsCount; ++i) {
		const Segment edge = {body->GetPointGlobal(i), body->GetPointGlobal((i + 1) % pointsCount)};

		if (auto i_point = FindSegmentCircleIntersectionPoint(edge, circle->getPosition(), circle->getRadius())) {
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

	return result;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectCircleCircleIntersection(const sf::CircleShape* circle1,
                                                                                    const sf::CircleShape* circle2) {
	using namespace Utils;
	auto r1 = circle1->getRadius();
	auto r2 = circle2->getRadius();
	auto pos1 = circle1->getPosition();
	auto pos2 = circle2->getPosition() - circle1->getPosition();
	float a = -2 * pos2.x;
	float b = -2 * pos2.y;
	float c = Sq(pos2.x) + Sq(pos2.y) + Sq(r1) - Sq(r2);
	auto p = Sq(c) - Sq(r1) * (Sq(a) + Sq(b));
	if (p > std::numeric_limits<float>::epsilon()) {
		return std::nullopt;
	}
	float x0 = -a * c / (Sq(a) + Sq(b)) + pos1.x;
	float y0 = -b * c / (Sq(a) + Sq(b)) + pos1.y;

	IntersectionDetails result;
	if (IsZero(p)) {
		result.intersection.start = sf::Vector2f(x0, y0);
		result.intersection.end = result.intersection.start;
	}
	else {
		float d = Sq(r1) - Sq(c) / (Sq(a) + Sq(b));
		float mult = sqrt(d / (Sq(a) + Sq(b)));
		result.intersection.start.x = x0 + b * mult;
		result.intersection.start.y = y0 - a * mult;
		result.intersection.end.x = x0 - b * mult;
		result.intersection.end.y = y0 + a * mult;
	}
	if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		if (auto wnd = Engine::MainContext::GetInstance().GetMainWindow()) {
			wnd->draw(CreateCircle(result.intersection.start, 2, sf::Color::White));
			wnd->draw(CreateCircle(result.intersection.end, 2, sf::Color::White));
		}
	}
	return result;
}

std::optional<SegmentIntersectionPoints> PhysicsProcessor::FindSegmentsIntersectionPoint(const Segment& segA,
                                                                                         const Segment& segB) {
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
		if (!Utils::ArePointsCollinear(segA.start, segA.end, segB.start)) {
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

			assert(!Utils::IsNan(intersectionSegment.start));
			assert(!Utils::IsNan(intersectionSegment.end));
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

			assert(!Utils::IsNan(intersectionSegment.start));
			assert(!Utils::IsNan(intersectionSegment.end));
		}
		return SegmentIntersectionPoints{intersectionSegment.start, intersectionSegment.end};
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
			return linesIntersection.y >= std::min(seg.start.y, seg.end.y) &&
			       linesIntersection.y <= std::max(seg.start.y, seg.end.y);
		}
		return linesIntersection.x >= std::min(seg.start.x, seg.end.x) &&
		       linesIntersection.x <= std::max(seg.start.x, seg.end.x);
	};
	if (!isInsideSegment(segA)) {
		return std::nullopt;
	}
	if (!isInsideSegment(segB)) {
		return std::nullopt;
	}
	return SegmentIntersectionPoints{linesIntersection};
}

std::optional<SegmentIntersectionPoints>
PhysicsProcessor::FindSegmentCircleIntersectionPoint(const Segment& seg, const sf::Vector2f& circleCenter,
                                                     float radius) {
	using namespace Utils;
	SegmentIntersectionPoints result;
	auto s = seg;
	s.start -= circleCenter;
	s.end -= circleCenter;

	auto v = seg.getDirVector();
	bool is_xy_swapped = false;
	if (IsZero(v.x)) {
		is_xy_swapped = true;
		std::swap(v.x, v.y);
		std::swap(s.start.x, s.start.y);
		std::swap(s.end.x, s.end.y);
	}

	if (s.start.x > s.end.x) {
		std::swap(s.start, s.end);
	}

	float slope = v.y / v.x;
	float a = Sq(slope) + 1;
	float b = slope * s.start.y - Sq(slope);
	float c = Sq(s.start.y) + Sq(slope) * s.start.x - s.start.x * s.start.y * slope - Sq(radius);

	auto x_resolution = SolveQuadraticEquation(a, b, c);
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

	if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		if (auto window = Engine::MainContext::GetInstance().GetMainWindow()) {
			window->draw(CreateCircle(result.p1, 2.f, sf::Color::White));
			if (result.p2) {
				window->draw(CreateCircle(*result.p2, 2.f, sf::Color::White));
			}
		}
	}
	return result;
}

void PhysicsProcessor::ResolveCollision(const IntersectionDetails& collision) {
	auto body1 = collision.wNode1.lock();
	auto body2 = collision.wNode2.lock();
	if (!body1 || !body2) {
		assert(false);
		return;
	}

	auto b1RigidBody = body1->RequireBehaviour<RigidBodyBehaviour>();
	auto b2RigidBody = body2->RequireBehaviour<RigidBodyBehaviour>();
	if (!b1RigidBody || !b2RigidBody) {
		assert(false);
		return;
	}

	if (b1RigidBody->IsImmovable()) { // fixed body must be second
		std::swap(body1, body2);
		std::swap(b1RigidBody, b2RigidBody);
	}

	const auto m1 = b1RigidBody->_mass;
	const auto m2 = b2RigidBody->_mass;
	auto v1_to_v2 = b2RigidBody->_velocity - b1RigidBody->_velocity;
	auto b1_tangent = collision.intersection.getDirVector();
	auto b1_normal = Utils::Normalize(sf::Vector2f(-b1_tangent.y, b1_tangent.x));
	if (Utils::Dot(body2->GetPosGlobal() - body1->GetPosGlobal(), b1_normal) < 0.f) {
		b1_tangent = -b1_tangent;
		b1_normal = -b1_normal;
	}

	/* overlapping fixing */
	auto getPenetrationDepth = [collisionPoint = collision.intersection.start](SceneNode* node,
	                                                                           const sf::Vector2f& bodyNormal) {
		float result = 0.f;
		auto* body = node->FindShapeCollider();
		if (!body) {
			return 0.f;
		}
		for (size_t i = 0; i < body->GetPointCount(); ++i) {
			auto penetrationVec = body->GetPointGlobal(i) - collisionPoint;
			float depth = Utils::Project(penetrationVec, bodyNormal);
			result = std::max(result, depth);
		}
		return result;
	};
	float penetrationDepthSum =
	    getPenetrationDepth(body1.get(), b1_normal) + getPenetrationDepth(body2.get(), -b1_normal);

	if (b2RigidBody->IsImmovable()) {
		auto b1_pos = body1->GetPosGlobal() - b1_normal * penetrationDepthSum;
		body1->SetPosGlobal(b1_pos);
	}
	else {
		auto b1_pos = body1->GetPosGlobal() - b1_normal * penetrationDepthSum * (m1 / (m1 + m2));
		body1->SetPosGlobal(b1_pos);
		auto b2_pos = body2->GetPosGlobal() + b1_normal * penetrationDepthSum * (m2 / (m1 + m2));
		body2->SetPosGlobal(b2_pos);
	}

	/* velocities handling */
	if (bool areMovingTowards = Utils::Dot(b1_normal, v1_to_v2) < 0.f) {
		const auto r = b1RigidBody->_restitution * b2RigidBody->_restitution;

		auto norm_v1 = Utils::Project(b1RigidBody->_velocity, b1_normal);
		auto norm_v2 = Utils::Project(b2RigidBody->_velocity, b1_normal);
		auto norm_v_diff = norm_v1 - norm_v2;

		float norm_dv1 = 0.f;
		float norm_dv2 = 0.f;

		if (b2RigidBody->IsImmovable()) {
			norm_dv1 = -(1.f + r) * norm_v_diff;
		}
		else {
			norm_dv1 = -(1.f + r) * m2 / (m1 + m2) * norm_v_diff;
			norm_dv2 = (1.f + r) * m1 / (m1 + m2) * norm_v_diff;
		}

		auto dv1 = b1_normal * norm_dv1;
		auto dv2 = b1_normal * norm_dv2;
		b1RigidBody->_velocity += dv1;
		b2RigidBody->_velocity += dv2;

		auto isNan = Utils::IsNan(b1RigidBody->_velocity) || Utils::IsNan(b2RigidBody->_velocity);
		assert(!isNan);

		if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
			if (auto window = Engine::MainContext::GetInstance().GetMainWindow()) {
				{
					VectorArrow force1(body1->GetPosGlobal(), body1->GetPosGlobal() + dv1);
					if (auto* collider = body1->FindShapeCollider()) {
						if (auto* shape = collider->GetBaseShape()) {
							auto color = shape->getFillColor();
							color.a = 255u;
							force1.SetColor(color);
						}
					}
					window->draw(force1);
				}
				{
					VectorArrow force2(body2->GetPosGlobal(), body2->GetPosGlobal() + dv2);
					if (auto* collider = body2->FindShapeCollider()) {
						if (auto* shape = collider->GetBaseShape()) {
							auto color = shape->getFillColor();
							color.a = 255u;
							force2.SetColor(color);
						}
					}
					window->draw(force2);
				}
			}
		}
	}

	if (Engine::MainContext::GetInstance().IsDebugDrawEnabled()) {
		if (auto window = Engine::MainContext::GetInstance().GetMainWindow()) {
			const sf::Vector2f middlePoint((collision.intersection.start + collision.intersection.end) * 0.5f);

			{ // collision segment
				VectorArrow segment(collision.intersection.start, collision.intersection.end, sf::Color::Magenta);
				window->draw(segment);
			}

			{ // normal
				VectorArrow tangentArrow(middlePoint, middlePoint + b1_normal * 50.f, sf::Color::Yellow);
				window->draw(tangentArrow);
			}

			window->draw(Utils::CreateCircle(middlePoint, 2, sf::Color::White));
		}
	}
}

const std::list<std::weak_ptr<SceneNode>>& PhysicsProcessor::GetAllBodies() const {
	return _bodies;
}

void PhysicsProcessor::SetGravity(const sf::Vector2f v) {
	_gravity = v;
}

sf::Vector2f PhysicsProcessor::GetGravity() const {
	return _gravity;
}

void PhysicsProcessor::SetGravityEnabled(bool enabled) {
	_isGravityEnabled = enabled;
}

bool PhysicsProcessor::IsGravityEnabled() const {
	return _isGravityEnabled;
}

std::shared_ptr<AttractionField> PhysicsProcessor::GetAttractionField() const {
	return _inverseSquareField;
}
