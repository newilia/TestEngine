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

namespace Utils {
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

	bool IsPointInsideOfTriangle(const sf::Vector2f& p, const sf::Vector2f& t1, const sf::Vector2f& t2,
	                             const sf::Vector2f& t3);

	/// Веерная триангуляция от первой вершины; Не для круга — см. `IsWorldPointInsideOfShape`.
	bool IsWorldPointInsideOfShapeByFan(const sf::Vector2f& worldPoint, const sf::Shape* shape);
	bool IsWorldPointInsideOfShape(const sf::Vector2f& worldPoint, const sf::Shape* shape);
	bool IsWorldPointInsideOfVisual(const sf::Vector2f& worldPoint, const Visual* visual);
	bool IsWorldPointInsideOfBody(const sf::Vector2f& worldPoint, const AbstractBody* body);

	bool IsNan(const sf::Vector2f& v);

	std::string ToString(const sf::Vector2f& v);

	sf::Vector2f FindCenterOfMass(const sf::Shape* shape);
	float CalcTriangleArea(float a, float b, float c); // Heron formula

	bool IsZero(float val);

	float Sq(float val);

	std::optional<std::pair<float, std::optional<float>>> SolveQuadraticEquation(float a, float b, float c);
	sf::CircleShape CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color);

	/// Системно разворачивает окно (Windows: `ShowWindow` + `SW_MAXIMIZE`); на других ОС — пусто.
	void MaximizeWindow(const sf::RenderWindow& window);

	template <typename T, typename U>
	std::shared_ptr<T> SharedPtrCast(const U* ptr) {
		return std::dynamic_pointer_cast<T>((const_cast<U*>(ptr))->shared_from_this());
	}

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixel);
	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2f& pixel);

} // namespace Utils
