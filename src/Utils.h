#pragma once
#include "SFML/Graphics.hpp"

namespace utils {
    float calcLength(const sf::Vector2f& vec);
	sf::Vector2f normalize(const sf::Vector2f& vec);
    float dot(const sf::Vector2f& a, const sf::Vector2f& b);
    sf::Vector2f reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector);
	bool arePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);
    void removeExpiredPointers(auto& containter) {
		auto [first, last] = std::ranges::remove_if(containter, [](auto ptr) {
			return ptr.expired();
		});
		containter.erase(first, last);
    }
}
