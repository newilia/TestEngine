#include "Utils.h"

float utils::length(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

sf::Vector2f utils::normalize(const sf::Vector2f& vec) {
	auto result = vec;
	if (auto length = utils::length(vec); length > std::numeric_limits<float>::epsilon()) {
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

float utils::project(const sf::Vector2f& a, const sf::Vector2f& b) {
	auto result = dot(a, b);
	if (auto lengthB = length(b); lengthB > std::numeric_limits<float>::epsilon()) {
		result /= length(b);
	}
	return result;
}

bool utils::arePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) {
	float triangleArea = 0.5f * ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y));
	return std::abs(triangleArea) <= std::numeric_limits<float>::epsilon();
}

bool utils::isPointInsideOfBody(const sf::Vector2f& point, const shared_ptr<AbstractBody>& body) {
	auto t1 = body->getPoint(0);
	for (int i = 0; i < body->getPointCount() - 2; ++i) {
		auto t2 = body->getPoint(i + 1);
		auto t3 = body->getPoint(i + 2);
		if (isPointInsideOfTriangle(point, t1, t2, t3)) {
			return true;
		}
	}
	return false;
}

bool utils::isPointInsideOfTriangle(sf::Vector2f p, sf::Vector2f t1, sf::Vector2f t2, sf::Vector2f t3) {
	auto a = (t1.x - p.x) * (t2.y - t1.y) - (t2.x - t1.x) * (t1.y - p.y);
	auto b = (t2.x - p.x) * (t3.y - t2.y) - (t3.x - t2.x) * (t2.y - p.y);
	auto c = (t3.x - p.x) * (t1.y - t3.y) - (t1.x - t3.x) * (t3.y - p.y);
	if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
		return true;
	}
	return false;
}

bool utils::isNan(const sf::Vector2f& point) {
	return std::isnan(point.x) || std::isnan(point.y);
}
