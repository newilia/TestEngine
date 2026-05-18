#include "PhysicsBodyBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/MathUtils.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "PhysicsBodyBehaviour.generated.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <variant>

namespace {

	float Cross2D(const sf::Vector2f& a, const sf::Vector2f& b) {
		return a.x * b.y - a.y * b.x;
	}

	sf::FloatRect LocalVertexBounds(const sf::Shape* shape) {
		sf::FloatRect bounds{};
		const std::size_t n = shape->getPointCount();
		if (n == 0) {
			return bounds;
		}
		const auto p0 = shape->getPoint(0);
		bounds.position = p0;
		bounds.size = {};
		for (std::size_t i = 1; i < n; ++i) {
			const auto p = shape->getPoint(i);
			const float minX = std::min(bounds.position.x, p.x);
			const float minY = std::min(bounds.position.y, p.y);
			const float maxX = std::max(bounds.position.x + bounds.size.x, p.x);
			const float maxY = std::max(bounds.position.y + bounds.size.y, p.y);
			bounds.position = {minX, minY};
			bounds.size = {maxX - minX, maxY - minY};
		}
		return bounds;
	}

	/// I_z / m in shape-local units for uniform-density lamina; convex polygon, fan triangles (v0, vi, vi+1).
	float PolygonIzPerMassFan(const sf::Shape* shape, const sf::Vector2f& comLocal) {
		const std::size_t n = shape->getPointCount();
		if (n < 3) {
			return 0.f;
		}
		const sf::Vector2f p0 = shape->getPoint(0);
		float totalArea = 0.f;
		for (std::size_t i = 1; i + 1 < n; ++i) {
			const sf::Vector2f p1 = shape->getPoint(i);
			const sf::Vector2f p2 = shape->getPoint(i + 1);
			totalArea += 0.5f * std::fabs(Cross2D(p1 - p0, p2 - p0));
		}
		constexpr float kAreaEps = 1e-12f;
		if (totalArea < kAreaEps) {
			return 0.f;
		}
		float izPerMass = 0.f;
		for (std::size_t i = 1; i + 1 < n; ++i) {
			const sf::Vector2f p1 = shape->getPoint(i);
			const sf::Vector2f p2 = shape->getPoint(i + 1);
			const float areaTri = 0.5f * std::fabs(Cross2D(p1 - p0, p2 - p0));
			const float frac = areaTri / totalArea;
			const float a = Utils::Length(p1 - p0);
			const float b = Utils::Length(p2 - p1);
			const float cLen = Utils::Length(p2 - p0);
			const sf::Vector2f cTri = (p0 + p1 + p2) / 3.f;
			const sf::Vector2f d = cTri - comLocal;
			const float d2 = Utils::Sq(d.x) + Utils::Sq(d.y);
			// Uniform triangle, I about its own COM, axis ⊥ plane: I = m (a²+b²+c²) / 36.
			izPerMass += frac * ((Utils::Sq(a) + Utils::Sq(b) + Utils::Sq(cLen)) / 36.f + d2);
		}
		return izPerMass;
	}

	float PolygonIzPerMassAabbFallback(const sf::Shape* shape) {
		const sf::FloatRect bounds = LocalVertexBounds(shape);
		const float hx = bounds.size.x * 0.5f;
		const float hy = bounds.size.y * 0.5f;
		return (Utils::Sq(hx) + Utils::Sq(hy)) / 3.f;
	}

} // namespace

void PhysicsBodyBehaviour::OnInit() {
	Behaviour::OnInit();

	_collisionGeomCacheOk = false;
	_collisionCacheShapePtr = nullptr;
	_inertiaWorldCacheValid = false;

	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->RegisterBody(shared_from_this());
	}
}

void PhysicsBodyBehaviour::OnDeinit() {
	Behaviour::OnDeinit();

	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->UnregisterBody(this);
	}
}

void PhysicsBodyBehaviour::OnAttached() {
	Behaviour::OnAttached();
	if (GetNode()->IsInited()) {
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			ph->RegisterBody(shared_from_this());
		}
	}
}

void PhysicsBodyBehaviour::OnDetached() {
	Behaviour::OnDetached();

	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->UnregisterBody(this);
	}
}

void PhysicsBodyBehaviour::OnEnabled(bool isEnabled) {
	_collisionGeomCacheOk = false;
	_collisionCacheShapePtr = nullptr;
	_inertiaWorldCacheValid = false;

	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		if (isEnabled) {
			ph->RegisterBody(shared_from_this());
		}
		else {
			ph->UnregisterBody(this);
		}
	}
}

const sf::Shape* PhysicsBodyBehaviour::GetColliderShape() const {
	if (auto node = GetNode()) {
		if (auto visual = node->GetVisual()) {
			if (auto shapeVisual = std::dynamic_pointer_cast<ShapeVisualBase>(visual)) {
				return shapeVisual->GetBaseShape();
			}
		}
	}
	return nullptr;
}

void PhysicsBodyBehaviour::SetFixed(bool isFixed) {
	_isFixed = isFixed;
	if (_isFixed) {
		_velocity = {};
		_angularSpeed = {};
	}
}

bool PhysicsBodyBehaviour::IsFixed() const {
	return _isFixed;
}

float PhysicsBodyBehaviour::GetMass() const {
	return _mass;
}

void PhysicsBodyBehaviour::SetMass(float m) {
	_mass = m;
	_inertiaWorldCacheValid = false;
}

sf::Vector2f PhysicsBodyBehaviour::GetVelocity() const {
	return _velocity;
}

void PhysicsBodyBehaviour::SetVelocity(sf::Vector2f v) {
	_velocity = v;
}

void PhysicsBodyBehaviour::AddVelocity(sf::Vector2f delta) {
	_velocity += delta;
}

float PhysicsBodyBehaviour::GetAngularSpeed() const {
	return _angularSpeed;
}

void PhysicsBodyBehaviour::SetAngularSpeed(float w) {
	_angularSpeed = w;
}

float PhysicsBodyBehaviour::GetRestitution() const {
	return _restitution;
}

void PhysicsBodyBehaviour::SetRestitution(float r) {
	_restitution = r;
}

float PhysicsBodyBehaviour::GetFriction() const {
	return _friction;
}

void PhysicsBodyBehaviour::SetFriction(float f) {
	_friction = f;
}

float PhysicsBodyBehaviour::GetGravityScale() const {
	return _gravityScale;
}

void PhysicsBodyBehaviour::SetGravityScale(float s) {
	_gravityScale = s;
}

PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetCollisionGroups() {
	return _collisionGroups;
}

const PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetCollisionGroups() const {
	return _collisionGroups;
}

PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetOverlapGroups() {
	return _overlapGroups;
}

const PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetOverlapGroups() const {
	return _overlapGroups;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetOnCollideSignal() const {
	return _onCollideSignal;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetOnOverlapSignal() const {
	return _onOverlapSignal;
}

float PhysicsBodyBehaviour::MinCollisionInertia(float mass) {
	return std::max(1e-6f, 1e-4f * std::max(mass, 1e-6f));
}

bool PhysicsBodyBehaviour::InertiaWorldCacheMatches(float mass, const sf::Transform& shapeToWorld) const {
	if (!_inertiaWorldCacheValid || mass != _cachedInertiaMass) {
		return false;
	}
	const float* m = shapeToWorld.getMatrix();
	for (unsigned i = 0; i < 16; ++i) {
		if (std::fabs(m[i] - _cachedInertiaShapeToWorldMatrix[i]) > 1e-4f) {
			return false;
		}
	}
	return true;
}

void PhysicsBodyBehaviour::StoreInertiaWorldCache(
    float mass, const sf::Transform& shapeToWorld, float inertiaWorld) const {
	_cachedInertiaWorld = inertiaWorld;
	_cachedInertiaMass = mass;
	const float* m = shapeToWorld.getMatrix();
	for (unsigned i = 0; i < 16; ++i) {
		_cachedInertiaShapeToWorldMatrix[i] = m[i];
	}
	_inertiaWorldCacheValid = true;
}

void PhysicsBodyBehaviour::RefreshCollisionShapeCache() {
	const sf::Shape* shape = GetColliderShape();
	if (!shape) {
		_collisionCacheShapePtr = nullptr;
		_collisionGeomCacheOk = false;
		_collisionGeomKind = CollisionGeomKind::None;
		_inertiaWorldCacheValid = false;
		return;
	}
	if (_collisionGeomCacheOk && _collisionCacheShapePtr == shape) {
		return;
	}

	_inertiaWorldCacheValid = false;
	_collisionCacheShapePtr = shape;
	_collisionComLocal = Utils::FindCenterOfMass(shape);

	if (const auto* circ = dynamic_cast<const sf::CircleShape*>(shape)) {
		_collisionGeomKind = CollisionGeomKind::Circle;
		_collisionCircleRadiusLocal = circ->getRadius();
		_collisionGeomCacheOk = true;
		return;
	}
	if (const auto* rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
		_collisionGeomKind = CollisionGeomKind::Rectangle;
		_collisionRectSize = rect->getSize();
		_collisionGeomCacheOk = true;
		return;
	}

	_collisionGeomKind = CollisionGeomKind::PolygonRough;
	const std::size_t n = shape->getPointCount();
	if (n < 3) {
		_collisionGeomCacheOk = false;
		return;
	}
	// Convex CCW outline: fan (v0, vi, vi+1) partitions the polygon. Concave / wrong winding may be wrong.
	_polygonIzPerMassLocal = PolygonIzPerMassFan(shape, _collisionComLocal);
	if (_polygonIzPerMassLocal < 1e-18f) {
		_polygonIzPerMassLocal = PolygonIzPerMassAabbFallback(shape);
	}
	_collisionGeomCacheOk = true;
}

sf::Vector2f PhysicsBodyBehaviour::GetCollisionComWorld(const sf::Transform& shapeToWorld) const {
	return shapeToWorld.transformPoint(_collisionComLocal);
}

float PhysicsBodyBehaviour::EstimateCollisionInertiaWorld(float mass, const sf::Transform& T) const {
	const float iMin = MinCollisionInertia(mass);
	if (!_collisionGeomCacheOk || mass <= 0.f) {
		return iMin;
	}
	if (InertiaWorldCacheMatches(mass, T)) {
		return _cachedInertiaWorld;
	}

	float result = iMin;
	switch (_collisionGeomKind) {
	case CollisionGeomKind::Circle: {
		const sf::Vector2f origin = T.transformPoint({0.f, 0.f});
		const float sx = Utils::Length(T.transformPoint({1.f, 0.f}) - origin);
		const float sy = Utils::Length(T.transformPoint({0.f, 1.f}) - origin);
		const float scale = 0.5f * (sx + sy);
		const float rw = _collisionCircleRadiusLocal * scale;
		result = std::max(iMin, 0.5f * mass * Utils::Sq(rw));
		break;
	}
	case CollisionGeomKind::Rectangle: {
		const sf::Vector2f cW = T.transformPoint(_collisionComLocal);
		const sf::Vector2f halfWx =
		    T.transformPoint(_collisionComLocal + sf::Vector2f{_collisionRectSize.x * 0.5f, 0.f}) - cW;
		const sf::Vector2f halfHy =
		    T.transformPoint(_collisionComLocal + sf::Vector2f{0.f, _collisionRectSize.y * 0.5f}) - cW;
		const float a2 = Utils::Sq(Utils::Length(halfWx));
		const float b2 = Utils::Sq(Utils::Length(halfHy));
		result = std::max(iMin, mass * (a2 + b2) / 3.f);
		break;
	}
	case CollisionGeomKind::PolygonRough: {
		const sf::Vector2f origin = T.transformPoint({0.f, 0.f});
		const float sx = Utils::Length(T.transformPoint({1.f, 0.f}) - origin);
		const float sy = Utils::Length(T.transformPoint({0.f, 1.f}) - origin);
		const float scale = 0.5f * (sx + sy);
		result = std::max(iMin, mass * _polygonIzPerMassLocal * Utils::Sq(scale));
		break;
	}
	case CollisionGeomKind::None:
	default:
		result = iMin;
		break;
	}

	StoreInertiaWorldCache(mass, T, result);
	return result;
}
