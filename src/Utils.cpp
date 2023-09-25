#include "Utils.h"
#include "fmt/format.h"
#include "SFML/Graphics.hpp"

float utils::length(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

float utils::manhattan_dist(const sf::Vector2f& vec) {
	return abs(vec.x) + abs(vec.y);
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

sf::Vector2f utils::rotate(const sf::Vector2f& v, float angle) {
	sf::Vector2f result;
	result.x = v.x * cos(angle) - v.y * sin(angle);
	result.y = v.x * sin(angle) + v.y * cos(angle);
	return result;
}

bool utils::isPointInsideOfBody(const sf::Vector2f& point, const shared_ptr<AbstractBody>& body) {
	auto t1 = body->getPointGlobal(0);
	for (size_t i = 0; i < body->getPointCount() - 2; ++i) {
		auto t2 = body->getPointGlobal(i + 1);
		auto t3 = body->getPointGlobal(i + 2);
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

bool utils::isNan(const sf::Vector2f& v) {
	return std::isnan(v.x) || std::isnan(v.y);
}

std::string utils::toString(const sf::Vector2f& v) {
	return fmt::format("({:.1f}, {:.1f})", v.x, v.y);
}

sf::Vector2f utils::findCenterOfMass(const sf::Shape* shape) {
	if (auto circle = dynamic_cast<const sf::CircleShape*>(shape)) {
		return sf::Vector2f(circle->getRadius(), circle->getRadius());
	}

	if (auto rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
		return sf::Vector2f(rect->getSize() * 0.5f);
	}

	auto pointCount = shape->getPointCount(); // single point and line
	if (pointCount < 3) {
		sf::Vector2f sum;
		for (size_t i = 0; i < pointCount; ++i) {
			sum += shape->getPoint(i);
		}
		return sum / static_cast<float>(pointCount);
	}

	{
		float trianglesAreaSum = 0.f;
		sf::Vector2f result;
		sf::Vector2f p1 = shape->getPoint(0);
		for (size_t i = 0; i < pointCount - 2; ++i) {
			sf::Vector2f p2 = shape->getPoint(i + 1);
			sf::Vector2f p3 = shape->getPoint(i + 2);
			auto a = length(p1 - p2);
			auto b = length(p2 - p3);
			auto c = length(p3 - p1);
			float triangleArea = calcTriangleArea(a, b, c);
			auto triangleCenter = (p1 + p2 + p3) / 3.f;
			trianglesAreaSum += triangleArea;
			result += triangleCenter * triangleArea;
		}
		return result / trianglesAreaSum;
	}
	
}

float utils::calcTriangleArea(float a, float b, float c) {
	float p = (a + b + c) * 0.5f; 
	return sqrt(p * (p - a) * (p - b) * (p - c));
}

std::optional<std::pair<float, std::optional<float>>> utils::solveQuadraticEquation(float a, float b, float c) {
	float D = sq(b) - 4 * a * c;
	if (D > std::numeric_limits<float>::epsilon()) {
		float x1 = (-b + sqrt(D)) / (2 * a);
		float x2 = (-b - sqrt(D)) / (2 * a);
		return std::pair(x1, x2);
	}
	if (isZero(D)) {
		return std::pair(-b / (2 * a), std::nullopt);
	}
	return std::nullopt;
}

sf::CircleShape utils::createCircle(const sf::Vector2f& pos, float radius, sf::Color color) {
	sf::CircleShape circle;
	circle.setPosition(pos);
	circle.setRadius(radius);
	circle.setOrigin({ radius, radius });
	circle.setFillColor(color);
	return circle;
}
