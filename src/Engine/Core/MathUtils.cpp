#include "Engine/Core/MathUtils.h"

#include "fmt/format.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cmath>

namespace Utils {
	float Length(const sf::Vector2f& vec) {
		return std::sqrt(vec.x * vec.x + vec.y * vec.y);
	}

	float ManhattanDist(const sf::Vector2f& vec) {
		return abs(vec.x) + abs(vec.y);
	}

	sf::Vector2f Normalize(const sf::Vector2f& vec) {
		auto result = vec;
		if (auto len = Length(vec); len > std::numeric_limits<float>::epsilon()) {
			result /= len;
		}
		return result;
	}

	float Dot(const sf::Vector2f& a, const sf::Vector2f& b) {
		return a.x * b.x + a.y * b.y;
	}

	sf::Vector2f Reflect(const sf::Vector2f& vector, const sf::Vector2f& relativeVector) {
		auto normal = Normalize(relativeVector);
		return vector - 2.f * normal * Dot(vector, normal);
	}

	float ScalarProjection(const sf::Vector2f& a, const sf::Vector2f& b) {
		auto result = Dot(a, b);
		if (auto lengthB = Length(b); lengthB > std::numeric_limits<float>::epsilon()) {
			result /= Length(b);
		}
		return result;
	}

	sf::Vector2f Projection(const sf::Vector2f& a, const sf::Vector2f& b) {
		return b * ScalarProjection(a, b);
	}

	bool ArePointsCollinear(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) {
		float triangleArea = 0.5f * ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y));
		return std::abs(triangleArea) <= std::numeric_limits<float>::epsilon();
	}

	sf::Vector2f Rotate(const sf::Vector2f& v, float angle) {
		sf::Vector2f result;
		result.x = v.x * cos(angle) - v.y * sin(angle);
		result.y = v.x * sin(angle) + v.y * cos(angle);
		return result;
	}

	bool IsPointInsideOfTriangle(
	    const sf::Vector2f& p, const sf::Vector2f& t1, const sf::Vector2f& t2, const sf::Vector2f& t3) {
		auto a = (t1.x - p.x) * (t2.y - t1.y) - (t2.x - t1.x) * (t1.y - p.y);
		auto b = (t2.x - p.x) * (t3.y - t2.y) - (t3.x - t2.x) * (t2.y - p.y);
		auto c = (t3.x - p.x) * (t1.y - t3.y) - (t1.x - t3.x) * (t3.y - p.y);
		if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
			return true;
		}
		return false;
	}

	bool IsNan(const sf::Vector2f& v) {
		return std::isnan(v.x) || std::isnan(v.y);
	}

	bool IsNan(const sf::FloatRect& rect) {
		return IsNan(rect.position) || IsNan(rect.size);
	}

	std::string ToString(const sf::Vector2f& v) {
		return fmt::format("({:.1f}, {:.1f})", v.x, v.y);
	}

	sf::Vector2f FindCenterOfMass(const sf::Shape* shape) {
		if (!shape) {
			return {};
		}

		if (auto circle = dynamic_cast<const sf::CircleShape*>(shape)) {
			return sf::Vector2f(circle->getRadius(), circle->getRadius());
		}

		if (auto rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
			return sf::Vector2f(rect->getSize() * 0.5f);
		}

		auto pointCount = shape->getPointCount();
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

	float CalcTriangleArea(float a, float b, float c) {
		float p = (a + b + c) * 0.5f;
		return sqrt(p * (p - a) * (p - b) * (p - c));
	}

	bool IsZero(float val) {
		return std::abs(val) <= std::numeric_limits<float>::epsilon();
	}

	float Sq(float val) {
		return val * val;
	}

	std::optional<std::pair<float, std::optional<float>>> SolveQuadraticEquation(float a, float b, float c) {
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

	namespace {
		float CrossOAB(const sf::Vector2f& O, const sf::Vector2f& A, const sf::Vector2f& B) {
			return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
		}
	} // namespace

	std::vector<sf::Vector2f> ConvexHull2D(std::vector<sf::Vector2f> points) {
		if (points.size() <= 1) {
			return points;
		}
		std::sort(points.begin(), points.end(), [](const sf::Vector2f& a, const sf::Vector2f& b) {
			return a.x < b.x || (a.x == b.x && a.y < b.y);
		});
		std::vector<sf::Vector2f> lower;
		lower.reserve(points.size());
		for (const auto& p : points) {
			while (lower.size() >= 2 && CrossOAB(lower[lower.size() - 2], lower[lower.size() - 1], p) <= 0.f) {
				lower.pop_back();
			}
			lower.push_back(p);
		}
		std::vector<sf::Vector2f> upper;
		upper.reserve(points.size());
		for (int i = static_cast<int>(points.size()) - 1; i >= 0; --i) {
			const auto& p = points[static_cast<std::size_t>(i)];
			while (upper.size() >= 2 && CrossOAB(upper[upper.size() - 2], upper[upper.size() - 1], p) <= 0.f) {
				upper.pop_back();
			}
			upper.push_back(p);
		}
		if (!lower.empty()) {
			lower.pop_back();
		}
		if (!upper.empty()) {
			upper.pop_back();
		}
		lower.insert(lower.end(), upper.begin(), upper.end());
		return lower;
	}
} // namespace Utils
