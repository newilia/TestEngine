#pragma once

#include "Engine/NodeVisual.h"

#include <SFML/Graphics/Shape.hpp>

class ShapeNodeVisual : public NodeVisual
{
public:
	explicit ShapeNodeVisual(sf::Shape* shape) : _shape(shape) {}

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Shape* _shape = nullptr;
};

inline void ShapeNodeVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_shape) {
		target.draw(*_shape, states);
	}
}
