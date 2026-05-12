#include "PhysicsProcessor.h"

#include "Engine/Behaviour/Physics/AttractiveBehaviour.h"
#include "Engine/Behaviour/Physics/IntersectionDetails.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/CircleShape.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <optional>
#include <vector>

namespace {

	sf::Vector2f WorldCircleCenter(const SceneNode& node, const sf::CircleShape& c) {
		sf::Transform full = node.GetWorldTransform();
		full *= c.getTransform();
		return full.transformPoint(c.getGeometricCenter());
	}

} // namespace

void PhysicsProcessor::RegisterBody(shared_ptr<PhysicsBodyBehaviour> body) {
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
	const float subDt = dt.asSeconds() / _simulationSubsteps;
	const float dampingFactor = _airFriction > 0.f ? std::exp(-_airFriction * subDt) : 1.f;

	for (int i = 0; i < _simulationSubsteps; ++i) {
		for (auto it = _bodies.begin(); it != _bodies.end();) {
			auto body = it->lock();
			if (!body) {
				it = _bodies.erase(it);
				continue;
			}
			MotionSubstep(body.get(), subDt, dampingFactor);
			++it;
		}

		DetactAndResolveCollisions();
	}
}

void PhysicsProcessor::MotionSubstep(PhysicsBodyBehaviour* body, float dtSec, float dampingFactor) {
	auto node = Verify(body->GetNode());
	if (!node) {
		return;
	}
	if (body->IsFixed()) {
		return;
	}

	if (_isGravityEnabled) {
		auto gravity = _gravity * body->GetGravityScale();
		body->AddVelocity(gravity * dtSec);
	}

	if (auto attractive = node->FindBehaviour<AttractiveBehaviour>()) {
		if (attractive->IsEnabled()) {
			sf::Vector2f a = _attractionField->EvaluateAcceleration(attractive);
			body->AddVelocity(a * dtSec);
		}
	}

	if (dampingFactor != 1.f) {
		auto dampedVel = body->GetVelocity() * dampingFactor;
		body->SetVelocity(dampedVel);
	}

	auto pos = Utils::GetWorldPos(node);
	pos += body->GetVelocity() * dtSec;
	Utils::SetLocalPosToWorld(node, pos);
}

void PhysicsProcessor::DetactAndResolveCollisions() {
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
		const bool interactionPair = (pb1->GetInteractionGroups() & pb2->GetInteractionGroups()).any();
		const bool overlapPair = (pb1->GetOverlappingGroups() & pb2->GetOverlappingGroups()).any();
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
			if (auto intersection = DetectIntersection(node1, node2, body1, body2)) {
				if ((body1->GetInteractionGroups() & body2->GetInteractionGroups()).any()) {
					if (!body1->IsFixed() || !body2->IsFixed()) {
						ResolveCollision(*intersection);
						body1->GetOnCollideSignal().Emit(*intersection);
						body2->GetOnCollideSignal().Emit(*intersection);
					}
				}

				if ((body1->GetOverlappingGroups() & body2->GetOverlappingGroups()).any()) {
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

	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);

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

std::optional<IntersectionDetails> PhysicsProcessor::DetectCirclePolygonIntersection(const SceneNode& circleNode,
    const sf::CircleShape* circle, const SceneNode& polygonNode, const sf::Shape* polygonShape) {
	std::vector<sf::Vector2f> edges_i_p;
	edges_i_p.reserve(2);

	const auto polygonPointsCount = polygonShape->getPointCount();
	IntersectionDetails result;

	const sf::Vector2f circleCenterWorld = WorldCircleCenter(circleNode, *circle);

	for (size_t i = 0; i < polygonPointsCount; ++i) {
		const Segment edge = {Utils::GetShapePointWorldPos(polygonShape, &polygonNode, i),
		    Utils::GetShapePointWorldPos(polygonShape, &polygonNode, (i + 1) % polygonPointsCount)};

		if (auto i_point = FindSegmentCircleIntersectionPoint(edge, circleCenterWorld, circle->getRadius())) {
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

std::optional<IntersectionDetails> PhysicsProcessor::DetectCircleCircleIntersection(
    const SceneNode& node1, const sf::CircleShape* circle1, const SceneNode& node2, const sf::CircleShape* circle2) {
	using namespace Utils;
	auto r1 = circle1->getRadius();
	auto r2 = circle2->getRadius();
	auto pos1 = WorldCircleCenter(node1, *circle1);
	auto pos2 = WorldCircleCenter(node2, *circle2) - pos1;
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

	return result;
}

void PhysicsProcessor::ResolveCollision(const IntersectionDetails& collision) {
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

	auto shape1 = pb1->GetColliderShape();
	auto shape2 = pb2->GetColliderShape();

	if (pb1->IsFixed()) { // fixed body must be second
		std::swap(body1, body2);
		std::swap(pb1, pb2);
		std::swap(shape1, shape2);
	}

	const auto m1 = pb1->GetMass();
	const auto m2 = pb2->GetMass();
	auto v1_to_v2 = pb2->GetVelocity() - pb1->GetVelocity();
	auto b1_tangent = collision.intersection.getDirVector();
	auto b1_normal = Utils::Normalize(sf::Vector2f(-b1_tangent.y, b1_tangent.x));
	if (Utils::Dot(Utils::GetWorldPos(body2) - Utils::GetWorldPos(body1), b1_normal) < 0.f) {
		b1_tangent = -b1_tangent;
		b1_normal = -b1_normal;
	}

	/* overlapping fixing */
	auto getPenetrationDepth = [collisionPoint = collision.intersection.start](
	                               sf::Shape const* shape, SceneNode const* node, const sf::Vector2f& bodyNormal) {
		float result = 0.f;
		auto* bodyBeh = node->FindBehaviour<PhysicsBodyBehaviour>().get();
		if (!bodyBeh) {
			return 0.f;
		}
		for (size_t i = 0; i < shape->getPointCount(); ++i) {
			auto penetrationVec = Utils::GetShapePointWorldPos(shape, node, i) - collisionPoint;
			float depth = Utils::Project(penetrationVec, bodyNormal);
			result = std::max(result, depth);
		}
		return result;
	};

	float penetrationDepthSum =
	    getPenetrationDepth(shape1, body1.get(), b1_normal) + getPenetrationDepth(shape2, body2.get(), -b1_normal);

	if (pb2->IsFixed()) {
		auto b1_pos = Utils::GetWorldPos(body1) - b1_normal * penetrationDepthSum;
		Utils::SetLocalPosToWorld(body1, b1_pos);
	}
	else {
		auto b1_pos = Utils::GetWorldPos(body1) - b1_normal * penetrationDepthSum * (m1 / (m1 + m2));
		Utils::SetLocalPosToWorld(body1, b1_pos);
		auto b2_pos = Utils::GetWorldPos(body2) + b1_normal * penetrationDepthSum * (m2 / (m1 + m2));
		Utils::SetLocalPosToWorld(body2, b2_pos);
	}

	/* velocities handling */
	if (bool areMovingTowards = Utils::Dot(b1_normal, v1_to_v2) < 0.f) {
		const auto r = pb1->GetRestitution() * pb2->GetRestitution();

		auto norm_v1 = Utils::Project(pb1->GetVelocity(), b1_normal);
		auto norm_v2 = Utils::Project(pb2->GetVelocity(), b1_normal);
		auto norm_v_diff = norm_v1 - norm_v2;

		float norm_dv1 = 0.f;
		float norm_dv2 = 0.f;

		if (pb2->IsFixed()) {
			norm_dv1 = -(1.f + r) * norm_v_diff;
		}
		else {
			norm_dv1 = -(1.f + r) * m2 / (m1 + m2) * norm_v_diff;
			norm_dv2 = (1.f + r) * m1 / (m1 + m2) * norm_v_diff;
		}

		auto dv1 = b1_normal * norm_dv1;
		auto dv2 = b1_normal * norm_dv2;
		pb1->AddVelocity(dv1);
		pb2->AddVelocity(dv2);

		auto isNan = Utils::IsNan(pb1->GetVelocity()) || Utils::IsNan(pb2->GetVelocity());
		assert(!isNan);
	}
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

void PhysicsProcessor::SetMotionSubsteps(int substeps) {
	_simulationSubsteps = std::clamp(substeps, 1, 10);
}

int PhysicsProcessor::GetMotionSubsteps() const {
	return _simulationSubsteps;
}
