#include "PhysicsBodyBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "PhysicsBodyBehaviour.generated.hpp"

#include <limits>
#include <memory>
#include <variant>

PhysicsBodyBehaviour::PhysicsBodyBehaviour() {}

PhysicsBodyBehaviour::~PhysicsBodyBehaviour() {
	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->UnregisterBody(this);
	}
}

void PhysicsBodyBehaviour::OnInit() {
	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->RegisterBody(shared_from_this());
	}
}

void PhysicsBodyBehaviour::OnDeinit() {
	if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		ph->UnregisterBody(this);
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
	const sf::Shape* s = GetShape();
	auto n = GetNode();
	if (!s || !n) {
		return {};
	}
	sf::Transform full = n->GetWorldTransform();
	full *= s->getTransform();
	return Utils::AxisAlignedBoundsAfterTransform(full, s->getLocalBounds());
}

size_t PhysicsBodyBehaviour::GetPointCount() const {
	return GetShape()->getPointCount();
}

sf::Vector2f PhysicsBodyBehaviour::GetPointWorldPos(std::size_t index) const {
	const auto* s = GetShape();
	auto n = GetNode();
	if (!s || !n) {
		return {};
	}
	sf::Transform full = n->GetWorldTransform();
	full *= s->getTransform();
	return full.transformPoint(s->getPoint(index));
}

void PhysicsBodyBehaviour::SetFixed(bool isFixed) {
	_isFixed = isFixed;
}

bool PhysicsBodyBehaviour::IsFixed() const {
	return _isFixed;
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

PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetInteractionGroups() {
	return _interactionGroups;
}

const PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetInteractionGroups() const {
	return _interactionGroups;
}

PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetOverlappingGroups() {
	return _overlappingGroups;
}

const PhysicsBodyBehaviour::GroupSet& PhysicsBodyBehaviour::GetOverlappingGroups() const {
	return _overlappingGroups;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetOnCollideSignal() const {
	return _onCollideSignal;
}

Signal<const IntersectionDetails&>& PhysicsBodyBehaviour::GetOnOverlapSignal() const {
	return _onOverlapSignal;
}
