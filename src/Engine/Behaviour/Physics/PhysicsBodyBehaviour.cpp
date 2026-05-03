#include "PhysicsBodyBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "PhysicsBodyBehaviour.generated.hpp"

#include <limits>
#include <memory>
#include <variant>

PhysicsBodyBehaviour::PhysicsBodyBehaviour() {}

PhysicsBodyBehaviour::~PhysicsBodyBehaviour() {
	if (auto n = GetNode()) {
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			ph->UnregisterBody(n.get());
		}
	}
}

void PhysicsBodyBehaviour::OnInit() {
	if (auto n = GetNode()) {
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			ph->RegisterBody(n);
		}
	}
}

void PhysicsBodyBehaviour::OnDeinit() {
	if (auto n = GetNode()) {
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			ph->UnregisterBody(n.get());
		}
	}
}

sf::Shape* PhysicsBodyBehaviour::GetShape() {
	if (auto node = GetNode()) {
		if (auto visual = node->GetVisual()) {
			if (auto shapeVisual = std::dynamic_pointer_cast<ShapeVisualBase>(visual)) {
				return shapeVisual->GetBaseShape();
			}
		}
	}
	return nullptr;
}

const sf::Shape* PhysicsBodyBehaviour::GetShape() const {
	if (auto node = GetNode()) {
		if (auto visual = node->GetVisual()) {
			if (auto shapeVisual = std::dynamic_pointer_cast<ShapeVisualBase>(visual)) {
				return shapeVisual->GetBaseShape();
			}
		}
	}
	return nullptr;
}

sf::FloatRect PhysicsBodyBehaviour::GetBbox() const {
	return GetShape()->getGlobalBounds();
}

size_t PhysicsBodyBehaviour::GetPointCount() const {
	return GetShape()->getPointCount();
}

sf::Vector2f PhysicsBodyBehaviour::GetPointGlobal(std::size_t index) const {
	const auto* s = GetShape();
	return s->getTransform().transformPoint(s->getPoint(index));
}

sf::Vector2f PhysicsBodyBehaviour::GetPosGlobal() const {
	return GetShape()->getPosition();
}

void PhysicsBodyBehaviour::SetPosGlobal(sf::Vector2f pos) {
	GetShape()->setPosition(pos);
}

void PhysicsBodyBehaviour::SetImmovable() {
	_mass = std::numeric_limits<float>::infinity();
}

bool PhysicsBodyBehaviour::IsImmovable() const {
	return _mass == std::numeric_limits<float>::infinity();
}

float PhysicsBodyBehaviour::GetMass() const {
	return _mass;
}

void PhysicsBodyBehaviour::SetMass(float m) {
	_mass = m;
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

void PhysicsBodyBehaviour::ScaleVelocity(float factor) {
	_velocity *= factor;
}

float PhysicsBodyBehaviour::GetAngle() const {
	return _angle;
}

void PhysicsBodyBehaviour::SetAngle(float a) {
	_angle = a;
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

PhysicsBodyBehaviour::CollisionGroups& PhysicsBodyBehaviour::GetCollisionGroups() {
	return _collisionGroups;
}

const PhysicsBodyBehaviour::CollisionGroups& PhysicsBodyBehaviour::GetCollisionGroups() const {
	return _collisionGroups;
}

PhysicsBodyBehaviour::CollisionGroups& PhysicsBodyBehaviour::GetOverlappingGroups() {
	return _overlappingGroups;
}

const PhysicsBodyBehaviour::CollisionGroups& PhysicsBodyBehaviour::GetOverlappingGroups() const {
	return _overlappingGroups;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetCollisionCallbacks() {
	return _collisionCallbacks;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetOverlappingCallbacks() {
	return _overlappingCallbacks;
}
