#pragma once

#include "SFML/Graphics.hpp"

#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Utils {
	float Length(const sf::Vector2f& vec);
	float ManhattanDist(const sf::Vector2f& vec);
	sf::Vector2f Normalize(const sf::Vector2f& vec);
	float Dot(const sf::Vector2f& a, const sf::Vector2f& b);
	sf::Vector2f Reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector);
	float ScalarProjection(const sf::Vector2f& a, const sf::Vector2f& b);
	sf::Vector2f Projection(const sf::Vector2f& a, const sf::Vector2f& b);
	bool ArePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3);

	sf::Vector2f Rotate(const sf::Vector2f& v, float angle);

	bool IsPointInsideOfTriangle(
	    const sf::Vector2f& p, const sf::Vector2f& t1, const sf::Vector2f& t2, const sf::Vector2f& t3);

	bool IsNan(const sf::Vector2f& v);
	bool IsNan(const sf::FloatRect& rect);

	std::string ToString(const sf::Vector2f& v);

	sf::Vector2f FindCenterOfMass(const sf::Shape* shape);
	float CalcTriangleArea(float a, float b, float c);

	bool IsZero(float val);

	float Sq(float val);

	std::optional<std::pair<float, std::optional<float>>> SolveQuadraticEquation(float a, float b, float c);

	/// Counter-clockwise convex hull; collinear boundary points may be omitted. Size may be 0..n.
	std::vector<sf::Vector2f> ConvexHull2D(std::vector<sf::Vector2f> points);
} // namespace Utils
