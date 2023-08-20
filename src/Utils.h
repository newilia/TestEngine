#pragma once
#include "PhysicsHandler.h"
#include "SFML/Graphics.hpp"

namespace utils {
    float length(const sf::Vector2f& vec);
	sf::Vector2f normalize(const sf::Vector2f& vec);
    float dot(const sf::Vector2f& a, const sf::Vector2f& b);
    sf::Vector2f reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector);
	float project(const sf::Vector2f& a, const sf::Vector2f& b);
	bool arePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);
    void removeExpiredPointers(auto& containter) {
		auto [first, last] = std::ranges::remove_if(containter, [](auto ptr) {
			return ptr.expired();
		});
		containter.erase(first, last);
    }

	bool isPointInsideOfBody(const sf::Vector2f& point, const shared_ptr<AbstractBody>& body);
	bool isPointInsideOfTriangle(sf::Vector2f p, sf::Vector2f t1, sf::Vector2f t2, sf::Vector2f t3);
}
