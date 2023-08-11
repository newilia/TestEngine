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

sf::Vector2f utils::reflect(const sf::Vector2f& a, const sf::Vector2f& b) {
	auto bNorm = normalize(b);
	return a - 2.f * bNorm * dot(a, bNorm);
}
