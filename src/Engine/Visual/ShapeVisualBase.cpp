#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RenderTarget.hpp>

ShapeVisualBase::ShapeVisualBase() {}

const sf::Shape* ShapeVisualBase::GetBaseShape() const {
	return const_cast<ShapeVisualBase*>(this)->GetBaseShape();
}

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (auto shape = GetBaseShape()) {
		target.draw(*shape, states);
	}
}
