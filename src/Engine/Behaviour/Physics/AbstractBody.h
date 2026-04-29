#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstddef>

/// Геометрия и позиция физического тела (реализуется коллайдером-поведением на ноде).
class AbstractBody
{
public:
	virtual ~AbstractBody() = default;
	virtual sf::FloatRect GetBbox() const = 0;
	virtual size_t GetPointCount() const = 0;
	virtual sf::Vector2f GetPointGlobal(std::size_t index) const = 0;
	virtual sf::Vector2f GetPosGlobal() const = 0;
	virtual void SetPosGlobal(sf::Vector2f pos) = 0;
};
