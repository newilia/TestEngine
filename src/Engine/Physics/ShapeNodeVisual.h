#pragma once

#include "Engine/Visual/NodeVisual.h"

#include <SFML/Graphics/Shape.hpp>
#include <memory>

namespace sf {
class CircleShape;
class RectangleShape;
class ConvexShape;
}

/// Общая отрисовка `sf::Shape*`; проверка попадания — в наследниках.
class ShapeNodeVisualBase : public NodeVisual
{
public:
	explicit ShapeNodeVisualBase(sf::Shape* shape) : _shape(shape) {}

	void Draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::Shape* GetShape() const { return _shape; }

private:
	sf::Shape* _shape = nullptr;
};

class CircleShapeNodeVisual : public ShapeNodeVisualBase
{
public:
	explicit CircleShapeNodeVisual(sf::CircleShape* circle);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::CircleShape* _circle = nullptr;
};

class RectangleShapeNodeVisual : public ShapeNodeVisualBase
{
public:
	explicit RectangleShapeNodeVisual(sf::RectangleShape* rect);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::RectangleShape* _rect = nullptr;
};

class ConvexShapeNodeVisual : public ShapeNodeVisualBase
{
public:
	explicit ConvexShapeNodeVisual(sf::ConvexShape* convex);

	bool HitTest(sf::Vector2f windowPosition) const override;

private:
	sf::ConvexShape* _convex = nullptr;
};

/// Любой другой подкласс `sf::Shape`: веер по вершинам (как у тела).
class PolygonShapeNodeVisual : public ShapeNodeVisualBase
{
public:
	explicit PolygonShapeNodeVisual(sf::Shape* shape);

	bool HitTest(sf::Vector2f windowPosition) const override;
};

std::shared_ptr<NodeVisual> MakeShapeNodeVisual(sf::Shape* shape);
