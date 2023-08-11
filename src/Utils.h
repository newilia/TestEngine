#pragma once
#include "SFML/Graphics.hpp"

namespace utils {
    float calcLength(const sf::Vector2f& vec);
	sf::Vector2f normalize(const sf::Vector2f& vec);
    float dot(const sf::Vector2f& a, const sf::Vector2f& b);
    sf::Vector2f reflect(const sf::Vector2f& a, const sf::Vector2f& b);

}
