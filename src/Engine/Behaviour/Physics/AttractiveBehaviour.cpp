#include "AttractiveBehaviour.h"

#include "AttractiveBehaviour.generated.hpp"
#include "Engine/App/MainContext.h"
#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/AttractionField.h"
#include "Engine/Simulation/PhysicsProcessor.h"

void AttractiveBehaviour::OnInit() {
	const auto node = GetNode();
	if (!node) {
		return;
	}
	_rigidBody = node->RequireBehaviour<PhysicsBodyBehaviour>();

	node->RequireBehaviour<PhysicsDebugBehaviour>();
	if (const auto self = node->FindBehaviour<AttractiveBehaviour>()) {
		_self = self;
		if (const auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
			if (auto field = ph->GetAttractionField()) {
				field->Register(self);
			}
		}
	}
}

void AttractiveBehaviour::OnDeinit() {
	if (const auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		if (auto field = ph->GetAttractionField()) {
			if (const auto self = _self.lock()) {
				field->Unregister(self);
			}
		}
	}
	_rigidBody.reset();
	_self.reset();
}

void AttractiveBehaviour::OnUpdate(const sf::Time& dt) {
	if (!_isEnabled) {
		return;
	}
	const auto self = _self.lock();
	if (!self) {
		return;
	}
	const auto rb = _rigidBody.lock();
	if (!rb || rb->IsImmovable() || !std::isfinite(rb->_mass) || rb->_mass <= 0.f) {
		return;
	}
	if (const auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
		if (auto field = ph->GetAttractionField()) {
			sf::Vector2f a = field->EvaluateAcceleration(self);
			const float sec = dt.asSeconds();
			if (sec > 0.f) {
				rb->_velocity += a * sec;
			}
		}
	}
}

bool AttractiveBehaviour::IsEnabled() const {
	return _isEnabled;
}

float AttractiveBehaviour::GetAttraction() const {
	return _attraction;
}

void AttractiveBehaviour::SetAttraction(float value) {
	_attraction = value;
}
