#include "Engine/Visual/ShapeVisualBase.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RenderTarget.hpp>

ShapeVisualBase::ShapeVisualBase() {}

const sf::Shape* ShapeVisualBase::GetBaseShape() const {
	return const_cast<ShapeVisualBase*>(this)->GetBaseShape();
}

bool ShapeVisualBase::HitTest(const sf::Vector2f& worldPoint) const {
	auto node = GetNode();
	const sf::Transform nodeWorld = node ? node->GetWorldTransform() : sf::Transform{};
	return Utils::IsWorldPointInsideOfShape(worldPoint, GetBaseShape(), nodeWorld);
}

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto shape = GetBaseShape()) {
		target.draw(*shape, states);
	}
}
