#pragma once

#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/Shape.hpp>
#include <memory>

namespace sf {
class CircleShape;
class RectangleShape;
class ConvexShape;
}

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

class CircleShapeVisual : public ShapeVisualBase
{
public:
	explicit CircleShapeVisual(sf::CircleShape* circle);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::CircleShape* _circle = nullptr;
};

class RectangleShapeVisual : public ShapeVisualBase
{
public:
	explicit RectangleShapeVisual(sf::RectangleShape* rect);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::RectangleShape* _rect = nullptr;
};

class ConvexShapeVisual : public ShapeVisualBase
{
public:
	explicit ConvexShapeVisual(sf::ConvexShape* convex);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::ConvexShape* _convex = nullptr;
};

/// Любой другой подкласс `sf::Shape`: веер по вершинам (как у тела).
class PolygonShapeVisual : public ShapeVisualBase
{
public:
	explicit PolygonShapeVisual(sf::Shape* shape);

	bool HitTest(sf::Vector2f windowPosition) const override;
};

std::shared_ptr<Visual> MakeShapeVisual(sf::Shape* shape);
