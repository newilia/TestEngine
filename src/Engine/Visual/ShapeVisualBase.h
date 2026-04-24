#pragma once

#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/Shape.hpp>

/// Общая отрисовка `sf::Shape*`; проверка попадания — в наследниках.
class ShapeVisualBase : public Visual
{
public:
	explicit ShapeVisualBase(sf::Shape* shape) : _shape(shape) {}

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::Shape* GetShape() const { return _shape; }

private:
	sf::Shape* _shape = nullptr;
};
