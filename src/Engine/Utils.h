#pragma once
#include <optional>

#include "Physics/AbstractBody.h"
#include "SFML/Graphics.hpp"

namespace utils {
    float length(const sf::Vector2f& vec);
	float manhattan_dist(const sf::Vector2f& vec);
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
	sf::Vector2f rotate(const sf::Vector2f& v, float angle);

	bool isPointInsideOfBody(const sf::Vector2f& point, const AbstractBody* body);
	bool isPointInsideOfTriangle(sf::Vector2f p, sf::Vector2f t1, sf::Vector2f t2, sf::Vector2f t3);

	bool isNan(const sf::Vector2f& v);

	std::string toString(const sf::Vector2f& v);

	sf::Vector2f findCenterOfMass(const sf::Shape* shape);
	float calcTriangleArea(float a, float b, float c); // Heron formula
	inline bool isZero(float val) {
		return std::abs(val) <= std::numeric_limits<float>::epsilon();
	}
	inline float sq(float val) { return val * val; }

	std::optional<std::pair<float, std::optional<float>>> solveQuadraticEquation(float a, float b, float c);
	sf::CircleShape createCircle(const sf::Vector2f& pos, float radius, sf::Color color);

	template<typename T, typename U>
	shared_ptr<T> sharedPtrCast(const U* ptr) { return dynamic_pointer_cast<T>((const_cast<U*>(ptr))->shared_from_this()); }
}
