#include "InverseSquareFieldSourceBehaviour.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Behaviour/Physics/PhysicsDebugBehaviour.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/IsotropicInverseSquareField.h"
#include "Engine/Simulation/PhysicsHandler.h"
#include "InverseSquareFieldSourceBehaviour_gen.hpp"

void InverseSquareFieldSourceBehaviour::OnInit() {
	const auto node = GetNode();
	if (!node) {
		return;
	}
	if (const auto rb = node->FindBehaviour<RigidBodyBehaviour>()) {
		_rigidBody = rb;
	}
	else {
		return;
	}
	node->RequireBehaviour<PhysicsDebugBehaviour>();
	if (const auto self = node->FindBehaviour<InverseSquareFieldSourceBehaviour>()) {
		_self = self;
		if (const auto ph = EngineContext::GetInstance().GetPhysicsHandler()) {
			if (auto field = ph->GetIsotropicInverseSquareField()) {
				field->Register(self);
			}
		}
	}
}

void InverseSquareFieldSourceBehaviour::OnDeinit() {
	if (const auto ph = EngineContext::GetInstance().GetPhysicsHandler()) {
		if (auto field = ph->GetIsotropicInverseSquareField()) {
			if (const auto self = _self.lock()) {
				field->Unregister(self);
			}
		}
	}
	_rigidBody.reset();
	_self.reset();
}

void InverseSquareFieldSourceBehaviour::OnUpdate(const sf::Time& dt) {
	(void)dt;
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
	if (const auto ph = EngineContext::GetInstance().GetPhysicsHandler()) {
		if (auto field = ph->GetIsotropicInverseSquareField()) {
			sf::Vector2f a = field->EvaluateAcceleration(self);
			const float sec = EngineContext::GetInstance().GetSimDt().asSeconds();
			if (sec > 0.f) {
				rb->_velocity += a * sec;
			}
		}
	}
}
