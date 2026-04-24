#pragma once

#include "ShapeColliderBehaviourBase.h"

#include "PhysicalBehaviour.h"
#include "ShapeVisualBehaviour.h"
#include "Engine/Core/SceneNode.h"

#include <SFML/Graphics.hpp>

#include <memory>

template <typename TShape>
class ShapeColliderBehaviour final : public ShapeColliderBehaviourBase
{
public:
	sf::Shape* GetBaseShape() const override { return const_cast<TShape*>(&_shape); }

private:
	TShape _shape;
};

using CircleBodyCollider = ShapeColliderBehaviour<sf::CircleShape>;
using RectangleBodyCollider = ShapeColliderBehaviour<sf::RectangleShape>;

template <typename TShape>
std::shared_ptr<SceneNode> CreateShapeBodyNode() {
	auto n = std::make_shared<SceneNode>();
	n->AddBehaviour(std::make_shared<ShapeColliderBehaviour<TShape>>());
	n->RequireEntity<PhysicalBehaviour>();
	n->AddBehaviour(std::make_shared<ShapeVisualBehaviour>());
	return n;
}
