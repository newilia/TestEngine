#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RenderTarget.hpp>

ShapeVisualBase::ShapeVisualBase(sf::Shape* shape) : _shape(shape) {}

sf::Shape* ShapeVisualBase::GetShape() const {
	return _shape;
}

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (GetShape()) {
		target.draw(*GetShape(), states);
	}
}
