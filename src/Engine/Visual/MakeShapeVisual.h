#pragma once

#include <memory>

class Visual;
namespace sf {
class Shape;
}

std::shared_ptr<Visual> MakeShapeVisual(sf::Shape* shape);
