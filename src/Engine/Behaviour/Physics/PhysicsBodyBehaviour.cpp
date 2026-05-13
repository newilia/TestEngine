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

void PhysicsBodyBehaviour::OnInit() {
	Behaviour::OnInit();

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

void PhysicsBodyBehaviour::OnEnabled(bool isEnabled) {
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
