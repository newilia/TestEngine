#pragma once
#include "Engine/Behaviour/Physics/AbstractBody.h"
#include "SFML/Graphics.hpp"

#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <string>

class Visual;

namespace utils {
	float Length(const sf::Vector2f& vec);
	float ManhattanDist(const sf::Vector2f& vec);
	sf::Vector2f Normalize(const sf::Vector2f& vec);
	float Dot(const sf::Vector2f& a, const sf::Vector2f& b);
	sf::Vector2f Reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector);
	float Project(const sf::Vector2f& a, const sf::Vector2f& b);
	bool ArePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);

	void RemoveExpiredPointers(auto& containter) {
		auto [first, last] = std::ranges::remove_if(containter, [](auto ptr) { return ptr.expired(); });
		containter.erase(first, last);
	}

	sf::Vector2f Rotate(const sf::Vector2f& v, float angle);

	bool IsPointInsideOfBody(const sf::Vector2f& point, const AbstractBody* body);
	bool IsPointInsideOfTriangle(sf::Vector2f p, sf::Vector2f t1, sf::Vector2f t2, sf::Vector2f t3);

	/// Веерная триангуляция от первой вершины в мировых координатах (как у тела). Не вызывать для круга — см.
	/// `IsPointInsideOfShape`.
	bool IsPointInsideShapeByFan(const sf::Vector2f& point, const sf::Shape* shape);

	/// Круг — только формула; прямоугольник — inverse transform + локальный rect; иначе веер.
	bool IsPointInsideOfShape(const sf::Vector2f& point, const sf::Shape* shape);

	bool IsPointInsideOfVisual(const sf::Vector2f& point, const Visual* visual);

	bool IsNan(const sf::Vector2f& v);

	std::string ToString(const sf::Vector2f& v);

	sf::Vector2f FindCenterOfMass(const sf::Shape* shape);
	float CalcTriangleArea(float a, float b, float c); // Heron formula

	inline bool IsZero(float val) {
		return std::abs(val) <= std::numeric_limits<float>::epsilon();
	}

	inline float Sq(float val) {
		return val * val;
	}

	std::optional<std::pair<float, std::optional<float>>> SolveQuadraticEquation(float a, float b, float c);
	sf::CircleShape CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color);

	template <typename T, typename U>
	std::shared_ptr<T> SharedPtrCast(const U* ptr) {
		return std::dynamic_pointer_cast<T>((const_cast<U*>(ptr))->shared_from_this());
	}
} // namespace utils
