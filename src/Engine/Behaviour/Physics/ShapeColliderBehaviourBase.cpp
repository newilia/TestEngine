#include "ShapeColliderBehaviourBase.h"

#include "Engine/App/EngineContext.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "ShapeColliderBehaviourBase.generated.hpp"

ShapeColliderBehaviourBase::~ShapeColliderBehaviourBase() {
	if (_registered) {
		if (auto n = GetNode()) {
			if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
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
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
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
		if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
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
