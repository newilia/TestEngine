#include "ShapeColliderBehaviourBase.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/Simulation/PhysicsHandler.h"
#include "ShapeColliderBehaviourBase_gen.hpp"

ShapeColliderBehaviourBase::~ShapeColliderBehaviourBase() {
	if (_registered) {
		if (auto n = GetNode()) {
			if (auto ph = EngineContext::Instance().GetPhysicsHandler()) {
				ph->UnregisterBody(n.get());
			}
		}
		_registered = false;
	}
}

void ShapeColliderBehaviourBase::OnInit() {
	if (_registered) {
		return;
	}
	if (auto n = GetNode()) {
		if (auto ph = EngineContext::Instance().GetPhysicsHandler()) {
			ph->RegisterBody(n);
			_registered = true;
		}
	}
}

void ShapeColliderBehaviourBase::OnDeinit() {
	if (!_registered) {
		return;
	}
	if (auto n = GetNode()) {
		if (auto ph = EngineContext::Instance().GetPhysicsHandler()) {
			ph->UnregisterBody(n.get());
		}
	}
	_registered = false;
}

sf::FloatRect ShapeColliderBehaviourBase::GetBbox() const {
	return GetBaseShape()->getGlobalBounds();
}

size_t ShapeColliderBehaviourBase::GetPointCount() const {
	return GetBaseShape()->getPointCount();
}

sf::Vector2f ShapeColliderBehaviourBase::GetPointGlobal(std::size_t index) const {
	auto* s = GetBaseShape();
	return s->getTransform().transformPoint(s->getPoint(index));
}

sf::Vector2f ShapeColliderBehaviourBase::GetPosGlobal() const {
	return GetBaseShape()->getPosition();
}

void ShapeColliderBehaviourBase::SetPosGlobal(sf::Vector2f pos) {
	GetBaseShape()->setPosition(pos);
}
