#include "PhysicsBodyBehaviour.h"

#include "Engine/App/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Simulation/PhysicsProcessor.h"
#include "Engine/Visual/MakeShapeVisual.h"
#include "PhysicsBodyBehaviour.generated.hpp"

#include <limits>
#include <memory>
#include <variant>

PhysicsBodyBehaviour::PhysicsBodyBehaviour() : _shape(sf::CircleShape(5.f)) {}

PhysicsBodyBehaviour::PhysicsBodyBehaviour(sf::CircleShape shape, bool attachVisualFromShape)
    : _attachVisualFromShape(attachVisualFromShape), _shape(std::move(shape)) {}

PhysicsBodyBehaviour::PhysicsBodyBehaviour(sf::RectangleShape shape, bool attachVisualFromShape)
    : _attachVisualFromShape(attachVisualFromShape), _shape(std::move(shape)) {}

PhysicsBodyBehaviour::PhysicsBodyBehaviour(sf::ConvexShape shape, bool attachVisualFromShape)
    : _attachVisualFromShape(attachVisualFromShape), _shape(std::move(shape)) {}

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
	if (_attachVisualFromShape) {
		auto node = GetNode();
		if (node) {
			if (auto visual = MakeShapeVisual(GetShape())) {
				node->SetVisual(std::move(visual));
				_ownsShapeVisual = true;
			}
		}
	}

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
	if (_ownsShapeVisual) {
		if (auto node = GetNode()) {
			node->SetVisual(nullptr);
		}
		_ownsShapeVisual = false;
	}

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
	return std::visit(
	    [](auto& s) -> sf::Shape* {
		    return &s;
	    },
	    _shape);
}

const sf::Shape* PhysicsBodyBehaviour::GetShape() const {
	return std::visit(
	    [](const auto& s) -> const sf::Shape* {
		    return &s;
	    },
	    _shape);
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

template <typename TShape>
std::shared_ptr<SceneNode> CreatePhysicsBodyNode(bool attachVisualFromShape) {
	auto n = std::make_shared<SceneNode>();
	n->AddBehaviour(std::make_shared<PhysicsBodyBehaviour>(TShape{}, attachVisualFromShape));
	return n;
}

template std::shared_ptr<SceneNode> CreatePhysicsBodyNode<sf::CircleShape>(bool);
template std::shared_ptr<SceneNode> CreatePhysicsBodyNode<sf::RectangleShape>(bool);
template std::shared_ptr<SceneNode> CreatePhysicsBodyNode<sf::ConvexShape>(bool);
