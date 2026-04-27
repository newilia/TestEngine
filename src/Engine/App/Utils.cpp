#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/TextVisual.h"
#include "SFML/Graphics.hpp"
#include "fmt/format.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>

namespace {
	template <typename GetVertex>
	bool pointInsideConvexFan(const sf::Vector2f& point, std::size_t count, GetVertex&& getVertex) {
		if (count < 3) {
			return false;
		}
		auto t1 = getVertex(0);
		for (std::size_t i = 0; i < count - 2; ++i) {
			if (Utils::IsPointInsideOfTriangle(point, t1, getVertex(i + 1), getVertex(i + 2))) {
				return true;
			}
		}
		return false;
	}
} // namespace

float Utils::Length(const sf::Vector2f& vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

float Utils::ManhattanDist(const sf::Vector2f& vec) {
	return abs(vec.x) + abs(vec.y);
}

sf::Vector2f Utils::Normalize(const sf::Vector2f& vec) {
	auto result = vec;
	if (auto len = Utils::Length(vec); len > std::numeric_limits<float>::epsilon()) {
		result /= len;
	}
	return result;
}

float Utils::Dot(const sf::Vector2f& a, const sf::Vector2f& b) {
	return a.x * b.x + a.y * b.y;
}

sf::Vector2f Utils::Reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector) {
	auto normal = Normalize(relativeVector);
	return vector - 2.f * normal * Dot(vector, normal);
}

float Utils::Project(const sf::Vector2f& a, const sf::Vector2f& b) {
	auto result = Dot(a, b);
	if (auto lengthB = Length(b); lengthB > std::numeric_limits<float>::epsilon()) {
		result /= Length(b);
	}
	return result;
}

bool Utils::ArePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) {
	float triangleArea = 0.5f * ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y));
	return std::abs(triangleArea) <= std::numeric_limits<float>::epsilon();
}

sf::Vector2f Utils::Rotate(const sf::Vector2f& v, float angle) {
	sf::Vector2f result;
	result.x = v.x * cos(angle) - v.y * sin(angle);
	result.y = v.x * sin(angle) + v.y * cos(angle);
	return result;
}

bool Utils::IsPointInsideShapeByFan(const sf::Vector2f& point, const sf::Shape* shape) {
	if (!shape) {
		return false;
	}
	return pointInsideConvexFan(point, shape->getPointCount(), [&](std::size_t i) {
		return shape->getTransform().transformPoint(shape->getPoint(i));
	});
}

bool Utils::IsPointInsideOfShape(const sf::Vector2f& point, const sf::Shape* shape) {
	if (!shape) {
		return false;
	}
	if (const auto* circle = dynamic_cast<const sf::CircleShape*>(shape)) {
		const auto& tf = circle->getTransform();
		const sf::Vector2f localCenter = circle->getGeometricCenter();
		const sf::Vector2f center = tf.transformPoint(localCenter);
		const float radius = Length(tf.transformPoint(localCenter + sf::Vector2f{circle->getRadius(), 0.f}) - center);
		return Sq(point.x - center.x) + Sq(point.y - center.y) <= Sq(radius);
	}
	if (const auto* rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
		const sf::Vector2f local = rect->getInverseTransform().transformPoint(point);
		const sf::FloatRect localBounds({0.f, 0.f}, rect->getSize());
		return localBounds.contains(local);
	}
	return IsPointInsideShapeByFan(point, shape);
}

bool Utils::IsPointInsideOfVisual(const sf::Vector2f& point, const Visual* visual) {
	if (!visual) {
		return false;
	}
	if (const auto* shapeVisual = dynamic_cast<const ShapeVisualBase*>(visual)) {
		return IsPointInsideOfShape(point, shapeVisual->GetShape());
	}
	if (const auto* fps = dynamic_cast<const TextVisual*>(visual)) {
		if (const sf::Text* text = fps->GetText()) {
			return text->getGlobalBounds().contains(point);
		}
		return false;
	}
	return false;
}

bool Utils::IsPointInsideOfBody(const sf::Vector2f& point, const AbstractBody* body) {
	if (!body) {
		return false;
	}
	if (const auto* collider = dynamic_cast<const ShapeColliderBehaviourBase*>(body)) {
		if (const sf::Shape* shape = collider->GetBaseShape()) {
			return IsPointInsideOfShape(point, shape);
		}
	}
	return pointInsideConvexFan(point, body->GetPointCount(), [&](std::size_t i) { return body->GetPointGlobal(i); });
}

bool Utils::IsPointInsideOfTriangle(sf::Vector2f p, sf::Vector2f t1, sf::Vector2f t2, sf::Vector2f t3) {
	auto a = (t1.x - p.x) * (t2.y - t1.y) - (t2.x - t1.x) * (t1.y - p.y);
	auto b = (t2.x - p.x) * (t3.y - t2.y) - (t3.x - t2.x) * (t2.y - p.y);
	auto c = (t3.x - p.x) * (t1.y - t3.y) - (t1.x - t3.x) * (t3.y - p.y);
	if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
		return true;
	}
	return false;
}

bool Utils::IsNan(const sf::Vector2f& v) {
	return std::isnan(v.x) || std::isnan(v.y);
}

std::string Utils::ToString(const sf::Vector2f& v) {
	return fmt::format("({:.1f}, {:.1f})", v.x, v.y);
}

sf::Vector2f Utils::FindCenterOfMass(const sf::Shape* shape) {
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
			auto a = Length(p1 - p2);
			auto b = Length(p2 - p3);
			auto c = Length(p3 - p1);
			float triangleArea = CalcTriangleArea(a, b, c);
			auto triangleCenter = (p1 + p2 + p3) / 3.f;
			trianglesAreaSum += triangleArea;
			result += triangleCenter * triangleArea;
		}
		return result / trianglesAreaSum;
	}
}

float Utils::CalcTriangleArea(float a, float b, float c) {
	float p = (a + b + c) * 0.5f;
	return sqrt(p * (p - a) * (p - b) * (p - c));
}

std::optional<std::pair<float, std::optional<float>>> Utils::SolveQuadraticEquation(float a, float b, float c) {
	float D = Sq(b) - 4 * a * c;
	if (D > std::numeric_limits<float>::epsilon()) {
		float x1 = (-b + sqrt(D)) / (2 * a);
		float x2 = (-b - sqrt(D)) / (2 * a);
		return std::pair(x1, x2);
	}
	if (IsZero(D)) {
		return std::pair(-b / (2 * a), std::nullopt);
	}
	return std::nullopt;
}

sf::CircleShape Utils::CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color) {
	sf::CircleShape circle;
	circle.setPosition(pos);
	circle.setRadius(radius);
	circle.setOrigin({radius, radius});
	circle.setFillColor(color);
	return circle;
}

void Utils::MaximizeWindow(sf::RenderWindow& window) {
#ifdef _WIN32
	ShowWindow(static_cast<HWND>(window.getNativeHandle()), SW_MAXIMIZE);
#endif
}
