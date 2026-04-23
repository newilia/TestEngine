#include "ShapeColliderBehaviourBase.h"

#include "PhysicsHandler.h"
#include "Engine/EngineInterface.h"

ShapeColliderBehaviourBase::~ShapeColliderBehaviourBase() {
	if (_registered) {
		if (auto n = GetNode()) {
			if (auto ph = EngineContext::Instance().GetPhysicsHandler()) {
				ph->UnregisterBody(n.get());
			}
		}
	}
}

void ShapeColliderBehaviourBase::OnAttached() {
	if (auto n = GetNode()) {
		if (auto ph = EngineContext::Instance().GetPhysicsHandler()) {
			ph->RegisterBody(n);
			_registered = true;
		}
	}
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
