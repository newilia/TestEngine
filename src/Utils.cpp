#include "Utils.h"

float utils::calcLength(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

sf::Vector2f utils::normalize(const sf::Vector2f& vec) {
	auto result = vec;
	if (auto length = calcLength(vec)) {
		result /= length;
	}
	return result;
}

float utils::dot(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.x + a.y * b.y;
}

sf::Vector2f utils::reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector) {
	auto normal = normalize(relativeVector);
	return vector - 2.f * normal * dot(vector, normal);
}

bool utils::arePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) {
	float triangleArea = 0.5f * ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y));
	return std::abs(triangleArea) < std::numeric_limits<float>::epsilon();
}