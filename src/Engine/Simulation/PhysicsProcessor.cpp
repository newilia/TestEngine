#include "PhysicsProcessor.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SceneNodeUtils.h"
#include "Engine/Core/Transform.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Angle.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <limits>
#include <optional>
#include <vector>

namespace {

	sf::Vector2f WorldCircleCenter(const SceneNode& node, const sf::CircleShape& c) {
		sf::Transform full = node.GetWorldTransform();
		full *= c.getTransform();
		return full.transformPoint(c.getGeometricCenter());
	}

	float Cross2D(const sf::Vector2f& a, const sf::Vector2f& b) {
		return a.x * b.y - a.y * b.x;
	}

	/// 2D scalar ω: in-plane velocity contribution ω×r = (-ω*r.y, ω*r.x).
	sf::Vector2f OmegaCrossR(float omega, const sf::Vector2f& r) {
		return {-omega * r.y, omega * r.x};
	}

	float ContactSeparationRate(float pairSoftness) {
		const float rigidRate = gCollisionTuning.rigidSeparationRate;
		if (pairSoftness <= 0.f) {
			return rigidRate;
		}
		const float softRate = gCollisionTuning.softSeparationRate;
		return rigidRate + (softRate - rigidRate) * pairSoftness;
	}

} // namespace

void PhysicsProcessor::RegisterBody(shared_ptr<PhysicsBodyBehaviour> body) {
	auto it = std::find_if(_bodies.begin(), _bodies.end(), [body](const std::weak_ptr<PhysicsBodyBehaviour>& w) {
		return w.lock().get() == body.get();
	});
	if (it != _bodies.end()) {
		return;
	}
	_bodies.emplace_back(body);
}

void PhysicsProcessor::UnregisterBody(PhysicsBodyBehaviour* body) {
	auto it = std::find_if(_bodies.begin(), _bodies.end(), [body](const std::weak_ptr<PhysicsBodyBehaviour>& w) {
		return w.lock().get() == body;
	});
	if (it != _bodies.end()) {
		_bodies.erase(it);
	}
}

void PhysicsProcessor::Update(const sf::Time& dt) {
	const float substepDt = dt.asSeconds() / _simulationSubsteps;

	for (int i = 0; i < _simulationSubsteps; ++i) {
		for (auto it = _bodies.begin(); it != _bodies.end();) {
			auto body = it->lock();
			if (!body) {
				it = _bodies.erase(it);
				continue;
			}
			IntergateVelocity(body.get(), substepDt);
			++it;
		}
		for (auto it = _bodies.begin(); it != _bodies.end(); ++it) {
			auto body = it->lock();
			IntegratePosition(body.get(), substepDt);
		}

		DetactAndResolveCollisions(substepDt);
	}
}

void PhysicsProcessor::IntergateVelocity(PhysicsBodyBehaviour* body, float dtSec) {
	if (body->IsFixed()) {
		return;
	}
	const auto force = EvaluateExternalForces(body);
	body->AddVelocity(force * dtSec);
	ApplyAngularDamping(body, dtSec);
}

void PhysicsProcessor::ApplyAngularDamping(PhysicsBodyBehaviour* body, float dtSec) {
	if (body->IsFixed()) {
		return;
	}
	const float k = body->GetAngularDamping();
	if (k <= 0.f) {
		return;
	}
	float w = body->GetAngularSpeed();
	if (w == 0.f) {
		return;
	}
	const float sign = w > 0.f ? 1.f : -1.f;
	const float dw = -sign * k * dtSec;
	if (std::abs(dw) >= std::abs(w)) {
		body->SetAngularSpeed(0.f);
	}
	else {
		body->SetAngularSpeed(w + dw);
	}
}

sf::Vector2f PhysicsProcessor::EvaluateExternalForces(PhysicsBodyBehaviour* body) const {
	sf::Vector2f result;
	auto node = Verify(body->GetNode());
	if (!node) {
		return result;
	}
	if (body->IsFixed()) {
		return result;
	}

	if (_isGravityEnabled) {
		result += _gravity * body->GetGravityScale();
	}
	if (auto attractive = node->FindBehaviour<AttractiveBehaviour>()) {
		if (attractive->IsEnabled()) {
			result += _attractionField->EvaluateForce(attractive);
		}
	}
	if (_airFriction != 0.f) {
		const auto vel = body->GetVelocity();
		if (vel.x + vel.y > 1e-10) {
			result -= vel.normalized() * vel.lengthSquared() * _airFriction;
		}
	}
	if (const float k = body->GetLinearDamping(); k > 0.f) {
		const sf::Vector2f vel = body->GetVelocity();
		const float speed = Utils::Length(vel);
		if (speed > 1e-10f) {
			// F = -k * mass * v/|v|  =>  a = -k * v/|v|  (constant deceleration magnitude)
			result -= Utils::Normalize(vel) * k;
		}
	}
	assert(!Utils::IsNan(result));
	return result;
}

void PhysicsProcessor::IntegratePosition(PhysicsBodyBehaviour* body, float dtSec) {
	auto node = body->GetNode();
	if (!node) {
		return;
	}
	auto pos = Utils::GetWorldPos(node);
	if (Utils::IsNan(pos)) {
		return;
	}
	if (body->IsFixed()) {
		return;
	}
	pos += body->GetVelocity() * dtSec;
	Utils::SetLocalPosToWorld(node, pos);

	node->SetLocalRotation(node->GetLocalRotation() + sf::radians(body->GetAngularSpeed() * dtSec));
}

void PhysicsProcessor::DetactAndResolveCollisions(float dtSec) {
	// handle intersections — sweep-and-prune broad phase, then masks + narrow phase
	struct BodySweepEntry
	{
		SceneNode* node = nullptr;
		PhysicsBodyBehaviour* body = nullptr;
		sf::FloatRect bb;
		/// Same ordering as iteration over `_bodies` in the old all-pairs loop (outer = first).
		size_t listOrder = 0;
	};

	auto pairNeedsNarrowPhase = [](const PhysicsBodyBehaviour* pb1, const PhysicsBodyBehaviour* pb2) {
		const bool interactionPair = (pb1->GetCollisionGroups() & pb2->GetCollisionGroups()).any();
		const bool overlapPair = (pb1->GetOverlapGroups() & pb2->GetOverlapGroups()).any();
		return interactionPair || overlapPair;
	};

	std::vector<BodySweepEntry> sweepEntries;
	sweepEntries.reserve(_bodies.size());
	size_t bodyListIndex = 0;
	for (auto& wBody : _bodies) {
		auto body = wBody.lock();
		if (!body) {
			++bodyListIndex;
			continue;
		}
		auto node = body->GetNode();
		if (!node) {
			++bodyListIndex;
			continue;
		}
		auto shape = body->GetColliderShape();
		if (!shape) {
			++bodyListIndex;
			continue;
		}
		auto bbox = node->GetWorldTransform().transformRect(shape->getGlobalBounds());
		if (Utils::IsNan(bbox)) {
			++bodyListIndex;
			continue;
		}
		sweepEntries.push_back({node.get(), body.get(), bbox, bodyListIndex});
		++bodyListIndex;
	}

	std::sort(sweepEntries.begin(), sweepEntries.end(), [](const BodySweepEntry& a, const BodySweepEntry& b) {
		return a.bb.position.x < b.bb.position.x;
	});

	std::deque<size_t> active;
	for (size_t i = 0; i < sweepEntries.size(); ++i) {
		static const auto getBbMaxX = [](const sf::FloatRect& bbox) {
			return bbox.position.x + bbox.size.x;
		};

		while (!active.empty() && getBbMaxX(sweepEntries[active.front()].bb) < sweepEntries[i].bb.position.x) {
			active.pop_front();
		}
		for (size_t aj : active) {
			auto& eA = sweepEntries[aj];
			auto& eB = sweepEntries[i];
			if (!eA.bb.findIntersection(eB.bb)) {
				continue;
			}
			if (!pairNeedsNarrowPhase(eA.body, eB.body)) {
				continue;
			}
			const bool aIsFirstInBodyList = eA.listOrder < eB.listOrder;
			auto* node1 = aIsFirstInBodyList ? eA.node : eB.node;
			auto* node2 = aIsFirstInBodyList ? eB.node : eA.node;
			auto* body1 = aIsFirstInBodyList ? eA.body : eB.body;
			auto* body2 = aIsFirstInBodyList ? eB.body : eA.body;
			if (!body1 || !body2) {
				continue;
			}
			if (body1->IsFixed() && body2->IsFixed()) {
				continue;
			}
			if (auto intersection = DetectIntersection(node1, node2, body1, body2)) {
				if ((body1->GetCollisionGroups() & body2->GetCollisionGroups()).any()) {
					if (!body1->IsFixed() || !body2->IsFixed()) {
						ResolveCollision(*intersection, dtSec);
						body1->GetOnCollideSignal().Emit(*intersection);
						body2->GetOnCollideSignal().Emit(*intersection);
					}
				}

				if ((body1->GetOverlapGroups() & body2->GetOverlapGroups()).any()) {
					body1->GetOnOverlapSignal().Emit(*intersection);
					body2->GetOnOverlapSignal().Emit(*intersection);
				}
			}
		}
		active.push_back(i);
	}
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectIntersection(
    SceneNode* node1, SceneNode* node2, PhysicsBodyBehaviour* body1, PhysicsBodyBehaviour* body2) {
	if (VerifyFalse(!node1 || !node2)) {
		return std::nullopt;
	}
	if (VerifyFalse(!body1 || !body2)) {
		return std::nullopt;
	}

	auto assignNodes = [&](IntersectionDetails& intersection) {
		intersection.wNode1 = node1->weak_from_this();
		intersection.wNode2 = node2->weak_from_this();
	};

	if (auto* circ1 = dynamic_cast<const sf::CircleShape*>(body1->GetColliderShape())) {
		if (auto* circ2 = dynamic_cast<const sf::CircleShape*>(body2->GetColliderShape())) {
			if (auto r = DetectCircleCircleIntersection(*node1, circ1, *node2, circ2)) {
				assignNodes(*r);
				return r;
			}
			return std::nullopt;
		}
		else if (auto r = DetectCirclePolygonIntersection(*node1, circ1, *node2, body2->GetColliderShape())) {
			assignNodes(*r);
			return r;
		}
		return std::nullopt;
	}
	if (auto* circ2 = dynamic_cast<const sf::CircleShape*>(body2->GetColliderShape())) {
		if (auto r = DetectCirclePolygonIntersection(*node2, circ2, *node1, body1->GetColliderShape())) {
			assignNodes(*r);
			return r;
		}
		return std::nullopt;
	}
	if (auto r = DetectPolygonPolygonIntersection(body1, body2)) {
		assignNodes(*r);
		return r;
	}
	return std::nullopt;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectPolygonPolygonIntersection(
    const PhysicsBodyBehaviour* body1, const PhysicsBodyBehaviour* body2) {
	auto shape1 = body1->GetColliderShape();
	auto shape2 = body2->GetColliderShape();
	auto node1 = body1->GetNode().get();
	auto node2 = body2->GetNode().get();

	std::vector<sf::Vector2f> intersectionPoints;
	intersectionPoints.reserve(2);

	const auto pointsCount1 = shape1->getPointCount();
	const auto pointsCount2 = shape2->getPointCount();
	IntersectionDetails result;

	for (size_t i = 0; i < pointsCount1; ++i) {
		const Segment edge1 = {Utils::GetShapePointWorldPos(shape1, node1, i),
		    Utils::GetShapePointWorldPos(shape1, node1, (i + 1) % pointsCount1)};

		for (size_t j = 0; j < pointsCount2; ++j) {
			const Segment edge2 = {Utils::GetShapePointWorldPos(shape2, node2, j),
			    Utils::GetShapePointWorldPos(shape2, node2, (j + 1) % pointsCount2)};

			if (auto i_point = FindSegmentsIntersectionPoint(edge1, edge2)) {
				intersectionPoints.emplace_back(i_point->p1);
				if (i_point->p2) {
					intersectionPoints.emplace_back(*i_point->p2);
				}
			}
		}
	}

	if (intersectionPoints.size() <= 1) {
		return std::nullopt;
	}
	else {
		result.intersection.start = *intersectionPoints.begin();
		result.intersection.end = *intersectionPoints.rbegin();
	}
	assert(result.intersection.start != result.intersection.end);
	return result;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectCirclePolygonIntersection(const SceneNode& circleNode,
    const sf::CircleShape* circle, const SceneNode& polygonNode, const sf::Shape* polygonShape) {
	IntersectionDetails result;
	std::vector<sf::Vector2f> intersectionPoints;
	intersectionPoints.reserve(2);
	const auto polygonPointsCount = polygonShape->getPointCount();
	const sf::Vector2f circleCenterWorld = WorldCircleCenter(circleNode, *circle);

	for (size_t i = 0; i < polygonPointsCount; ++i) {
		const Segment edge = {Utils::GetShapePointWorldPos(polygonShape, &polygonNode, i),
		    Utils::GetShapePointWorldPos(polygonShape, &polygonNode, (i + 1) % polygonPointsCount)};

		if (auto i_point = FindSegmentCircleIntersectionPoint(edge, circleCenterWorld, circle->getRadius())) {
			intersectionPoints.emplace_back(i_point->p1);
			if (i_point->p2) {
				intersectionPoints.emplace_back(*i_point->p2);
			}
		}
	}

	if (intersectionPoints.size() <= 1) {
		return std::nullopt;
	}

	result.intersection.start = *intersectionPoints.begin();
	result.intersection.end = *intersectionPoints.rbegin();
	assert(result.intersection.start != result.intersection.end);
	return result;
}

std::optional<IntersectionDetails> PhysicsProcessor::DetectCircleCircleIntersection(
    const SceneNode& node1, const sf::CircleShape* circle1, const SceneNode& node2, const sf::CircleShape* circle2) {
	using namespace Utils;
	const float r1 = circle1->getRadius();
	const float r2 = circle2->getRadius();
	const sf::Vector2f pos1 = WorldCircleCenter(node1, *circle1);
	const sf::Vector2f centerOffset = WorldCircleCenter(node2, *circle2) - pos1;

	const float distSq = Sq(centerOffset.x) + Sq(centerOffset.y);
	// Radical-axis math uses denom = 4 * distSq; skip concentric / near-concentric (denom → 0, radii match → non-finite).
	constexpr float kCoincidentCentersRelSq = 1e-12f;
	if (distSq <= kCoincidentCentersRelSq * Sq(std::max(r1 + r2, 1e-20f))) {
		return std::nullopt;
	}

	const float centerDist = std::sqrt(distSq);
	const float sumR = r1 + r2;
	const float diffR = std::fabs(r1 - r2);
	const float sumScale = std::max(sumR, 1e-6f);
	const float sepEps = std::numeric_limits<float>::epsilon() * std::max(8.f * sumScale, 1.f);
	if (centerDist > sumR + sepEps) {
		return std::nullopt;
	}
	if (centerDist + sepEps < diffR) {
		return std::nullopt;
	}

	const float a = -2.f * centerOffset.x;
	const float b = -2.f * centerOffset.y;
	const float c = distSq + Sq(r1) - Sq(r2);
	const float denom = Sq(a) + Sq(b);

	const float p = Sq(c) - Sq(r1) * denom;
	const float pClampTol = std::numeric_limits<float>::epsilon() * std::max({std::fabs(Sq(c)), Sq(r1) * denom, 1.f});
	if (p > pClampTol) {
		return std::nullopt;
	}

	const float invDenom = 1.f / denom;
	const float x0 = -a * c * invDenom + pos1.x;
	const float y0 = -b * c * invDenom + pos1.y;
	const float halfChord = std::sqrt(std::max(0.f, -p)) / denom;
	if (halfChord <= std::numeric_limits<float>::epsilon()) {
		return std::nullopt;
	}

	IntersectionDetails result;
	result.intersection.start = {x0 + b * halfChord, y0 - a * halfChord};
	result.intersection.end = {x0 - b * halfChord, y0 + a * halfChord};
	assert(result.intersection.start != result.intersection.end);
	return result;
}

std::optional<SegmentIntersectionPoints> PhysicsProcessor::FindSegmentsIntersectionPoint(
    const Segment& segA, const Segment& segB) {
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

std::optional<SegmentIntersectionPoints> PhysicsProcessor::FindSegmentCircleIntersectionPoint(
    const Segment& seg, const sf::Vector2f& circleCenter, float radius) {
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
	float x0 = s.start.x;
	float y0 = s.start.y;
	float c_line = y0 - slope * x0;

	float a = 1.f + Sq(slope);
	float b = 2.f * slope * c_line;
	float c = Sq(c_line) - Sq(radius);

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

	return result;
}

void PhysicsProcessor::ResolveCollision(const IntersectionDetails& collision, float dtSec) {
	auto body1 = collision.wNode1.lock();
	auto body2 = collision.wNode2.lock();
	if (!body1 || !body2) {
		assert(false);
		return;
	}

	auto pb1 = body1->FindBehaviour<PhysicsBodyBehaviour>();
	auto pb2 = body2->FindBehaviour<PhysicsBodyBehaviour>();
	if (!pb1 || !pb2) {
		assert(false);
		return;
	}

	if (pb1->IsFixed() && !pb2->IsFixed()) {
		// fixed body must be second one
		std::swap(body1, body2);
		std::swap(pb1, pb2);
	}

	auto shape1 = pb1->GetColliderShape();
	auto shape2 = pb2->GetColliderShape();
	if (!shape1 || !shape2) {
		return;
	}
	pb1->RefreshCollisionShapeCache();
	pb2->RefreshCollisionShapeCache();

	const auto m1 = pb1->GetMass();
	const auto m2 = pb2->GetMass();

	assert(collision.intersection.getDirVector() != sf::Vector2f{});

	auto tangent = collision.intersection.getDirVector().normalized();
	auto normal = Utils::Normalize(sf::Vector2f(-tangent.y, tangent.x));
	if (Utils::Dot(Utils::GetWorldPos(body2) - Utils::GetWorldPos(body1), normal) < 0.f) {
		tangent = -tangent;
		normal = -normal;
	}

	/* displacement */
	const sf::Vector2f collisionPoint = (collision.intersection.start + collision.intersection.end) * 0.5f;
	auto getPenetrationDepth = [collisionPoint](
	                               sf::Shape const* shape, SceneNode const* node, const sf::Vector2f& bodyNormal) {
		auto* bodyBeh = node->FindBehaviour<PhysicsBodyBehaviour>().get();
		if (!bodyBeh) {
			return 0.f;
		}
		// Circle–circle / circle–polygon (circle side): farthest point along bodyNormal is on the
		// geometric circle, not on SFML's tessellated outline.
		if (const auto* circ = dynamic_cast<const sf::CircleShape*>(shape)) {
			return Utils::ScalarProjection(WorldCircleCenter(*node, *circ) - collisionPoint, bodyNormal) +
			       circ->getRadius();
		}
		// Polygon–polygon / circle–polygon (polygon side): for a convex outline the support map in
		// direction bodyNormal hits a vertex (same as max over tessellation points).
		float result = 0.f;
		for (size_t i = 0; i < shape->getPointCount(); ++i) {
			const auto p = Utils::GetShapePointWorldPos(shape, node, i);
			const auto penetrationVec = p - collisionPoint;
			auto depth = Utils::ScalarProjection(penetrationVec, bodyNormal);
			result = std::max(result, depth);
		}
		return result;
	};

	const float pairSoft = std::max(pb1->GetSoftness(), pb2->GetSoftness());
	const float fullPenetration =
	    getPenetrationDepth(shape1, body1.get(), normal) + getPenetrationDepth(shape2, body2.get(), -normal);

	if (pairSoft <= 0.f && fullPenetration > 0.f) {
		const float displacementDistance = fullPenetration * gCollisionTuning.displacementFactor;
		if (pb2->IsFixed()) {
			auto b1_pos = Utils::GetWorldPos(body1) - normal * displacementDistance;
			Utils::SetLocalPosToWorld(body1, b1_pos);
		}
		else {
			auto b1_pos = Utils::GetWorldPos(body1) - normal * displacementDistance * (m1 / (m1 + m2));
			Utils::SetLocalPosToWorld(body1, b1_pos);
			auto b2_pos = Utils::GetWorldPos(body2) + normal * displacementDistance * (m2 / (m1 + m2));
			Utils::SetLocalPosToWorld(body2, b2_pos);
		}
	}

	/* reaction — normal impulse at contact (linear + angular, 2D rigid) */
	const sf::Vector2f contact = collisionPoint;

	const sf::Transform shapeToWorld1 = body1->GetWorldTransform() * shape1->getTransform();
	const sf::Transform shapeToWorld2 = body2->GetWorldTransform() * shape2->getTransform();
	const sf::Vector2f comWorld1 = pb1->GetCollisionComWorld(shapeToWorld1);
	const sf::Vector2f comWorld2 = pb2->GetCollisionComWorld(shapeToWorld2);

	const float I1 = pb1->EstimateCollisionInertiaWorld(m1, shapeToWorld1);
	const float I2 = pb2->EstimateCollisionInertiaWorld(m2, shapeToWorld2);

	const float invM1 = (pb1->IsFixed() || m1 <= 0.f) ? 0.f : 1.f / m1;
	const float invM2 = (pb2->IsFixed() || m2 <= 0.f) ? 0.f : 1.f / m2;
	const float invI1 = (pb1->IsFixed() || I1 <= 0.f) ? 0.f : 1.f / I1;
	const float invI2 = (pb2->IsFixed() || I2 <= 0.f) ? 0.f : 1.f / I2;

	const sf::Vector2f r1 = contact - comWorld1;
	const sf::Vector2f r2 = contact - comWorld2;

	const float w1 = pb1->IsFixed() ? 0.f : pb1->GetAngularSpeed();
	const float w2 = pb2->IsFixed() ? 0.f : pb2->GetAngularSpeed();

	const sf::Vector2f v1 = pb1->GetVelocity();
	const sf::Vector2f v2 = pb2->GetVelocity();
	const sf::Vector2f v1p = v1 + OmegaCrossR(w1, r1);
	const sf::Vector2f v2p = v2 + OmegaCrossR(w2, r2);
	const sf::Vector2f vRel = v2p - v1p;
	const float vn = Utils::Dot(vRel, normal);
	const float approachSpeed = std::max(0.f, -vn);

	const float rn1 = Cross2D(r1, normal);
	const float rn2 = Cross2D(r2, normal);
	const float denom = invM1 + invM2 + rn1 * rn1 * invI1 + rn2 * rn2 * invI2;

	const bool isImpact = approachSpeed > gCollisionTuning.restingSpeedThreshold;
	const bool isRigidImpact = isImpact && pairSoft <= 0.f;
	const bool isSoftImpact = isImpact && pairSoft > 0.f;

	float appliedJnMag = 0.f;
	auto applyNormalImpulse = [&](float J) {
		appliedJnMag += std::abs(J);
		const sf::Vector2f impulse = normal * J;
		if (invM1 > 0.f) {
			pb1->AddVelocity(-impulse * invM1);
			pb1->SetAngularSpeed(pb1->GetAngularSpeed() - J * rn1 * invI1);
		}
		if (invM2 > 0.f) {
			pb2->AddVelocity(impulse * invM2);
			pb2->SetAngularSpeed(pb2->GetAngularSpeed() + J * rn2 * invI2);
		}
	};

	if (denom > std::numeric_limits<float>::epsilon()) {
		if (pairSoft > 0.f && fullPenetration > 0.f) {
			/* soft contact — velocity bias spring (rate * penetration), not impulse * dt */
			const float rate = ContactSeparationRate(pairSoft);
			const float vn_bias = rate * fullPenetration;
			const float e = isSoftImpact ? std::min(pb1->GetRestitution(), pb2->GetRestitution()) : 0.f;
			float J = 0.f;
			if (vn < 0.f) {
				J = -((1.f + e) * vn - vn_bias) / denom;
			}
			else if (vn < vn_bias) {
				J = -(vn - vn_bias) / denom;
			}
			if (J != 0.f) {
				applyNormalImpulse(J);
			}
		}
		else if (vn < 0.f) {
			const float e = std::min(pb1->GetRestitution(), pb2->GetRestitution());
			const float J = -(1.f + e) * vn / denom;
			applyNormalImpulse(J);
		}
	}

	if (!isRigidImpact && appliedJnMag > std::numeric_limits<float>::epsilon()) {
		/* tangential (Coulomb) friction impulse — removes slip at contact, drives spin via r×t */
		const float w1f = pb1->IsFixed() ? 0.f : pb1->GetAngularSpeed();
		const float w2f = pb2->IsFixed() ? 0.f : pb2->GetAngularSpeed();
		const sf::Vector2f v1f = pb1->GetVelocity();
		const sf::Vector2f v2f = pb2->GetVelocity();
		const sf::Vector2f v1pf = v1f + OmegaCrossR(w1f, r1);
		const sf::Vector2f v2pf = v2f + OmegaCrossR(w2f, r2);
		const sf::Vector2f vRelf = v2pf - v1pf;
		const float vt = Utils::Dot(vRelf, tangent);

		const float rt1 = Cross2D(r1, tangent);
		const float rt2 = Cross2D(r2, tangent);
		const float denomT = invM1 + invM2 + rt1 * rt1 * invI1 + rt2 * rt2 * invI2;
		if (denomT > std::numeric_limits<float>::epsilon()) {
			const float mu = std::min(pb1->GetFriction(), pb2->GetFriction());
			const float frictionBlend = isSoftImpact ? pairSoft : 1.f;
			const float JtFree = -vt / denomT;
			const float JtMax = mu * frictionBlend * appliedJnMag;
			const float Jt = std::clamp(JtFree, -JtMax, JtMax);
			const sf::Vector2f impT = tangent * Jt;
			if (invM1 > 0.f) {
				pb1->AddVelocity(-impT * invM1);
				pb1->SetAngularSpeed(pb1->GetAngularSpeed() - Jt * rt1 * invI1);
			}
			if (invM2 > 0.f) {
				pb2->AddVelocity(impT * invM2);
				pb2->SetAngularSpeed(pb2->GetAngularSpeed() + Jt * rt2 * invI2);
			}
		}
	}

	assert(!Utils::IsNan(pb1->GetVelocity()) && !Utils::IsNan(pb2->GetVelocity()));
}

const std::list<std::weak_ptr<PhysicsBodyBehaviour>>& PhysicsProcessor::GetAllBodies() const {
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

void PhysicsProcessor::SetAirFriction(float airFriction) {
	_airFriction = std::max(0.f, airFriction);
}

float PhysicsProcessor::GetAirFriction() const {
	return _airFriction;
}

std::shared_ptr<AttractionField> PhysicsProcessor::GetAttractionField() const {
	return _attractionField;
}

void PhysicsProcessor::SetSimulationSubsteps(int substeps) {
	_simulationSubsteps = substeps;
}

int PhysicsProcessor::GetSimulationSubsteps() const {
	return _simulationSubsteps;
}
