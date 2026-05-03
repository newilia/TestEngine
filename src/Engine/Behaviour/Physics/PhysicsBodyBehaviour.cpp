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
	if (_registered) {
		if (auto n = GetNode()) {
			if (auto ph = Engine::MainContext::GetInstance().GetPhysicsProcessor()) {
				ph->UnregisterBody(n.get());
			}
		}
		_registered = false;
	}
}

void PhysicsBodyBehaviour::OnInit() {
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

void PhysicsBodyBehaviour::OnDeinit() {
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
