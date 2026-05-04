#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "SFML/Graphics.hpp"
#include "fmt/format.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <vector>

namespace {
	template <typename GetVertex>
	bool pointInsideConvexFan(const sf::Vector2f& worldPoint, std::size_t count, GetVertex&& getVertex) {
		if (count < 3) {
			return false;
		}
		auto t1 = getVertex(0);
		for (std::size_t i = 0; i < count - 2; ++i) {
			if (Utils::IsPointInsideOfTriangle(worldPoint, t1, getVertex(i + 1), getVertex(i + 2))) {
				return true;
			}
		}
		return false;
	}

	const sf::Image& TexturePixelsForHitTest(const sf::Texture& texture) {
		static std::unordered_map<const sf::Texture*, sf::Image> cache;
		const auto [it, inserted] = cache.try_emplace(&texture);
		if (inserted) {
			it->second = texture.copyToImage();
		}
		return it->second;
	}

	bool IsWorldPointInsideSpriteConsideringAlpha(const sf::Vector2f& worldPoint, const sf::Sprite& sprite,
	                                              const sf::Transform& nodeWorld) {
		sf::Transform full = nodeWorld;
		full *= sprite.getTransform();
		const sf::Vector2f local = full.getInverse().transformPoint(worldPoint);
		const sf::FloatRect bounds = sprite.getLocalBounds();
		if (!bounds.contains(local)) {
			return false;
		}

		const sf::IntRect tr = sprite.getTextureRect();
		if (tr.size.x <= 0 || tr.size.y <= 0) {
			return false;
		}

		const float minX = std::min(bounds.position.x, bounds.position.x + bounds.size.x);
		const float minY = std::min(bounds.position.y, bounds.position.y + bounds.size.y);
		const float spanX = std::abs(bounds.size.x);
		const float spanY = std::abs(bounds.size.y);
		if (spanX <= std::numeric_limits<float>::epsilon() || spanY <= std::numeric_limits<float>::epsilon()) {
			return false;
		}

		const float nx = (local.x - minX) / spanX;
		const float ny = (local.y - minY) / spanY;

		int ox = static_cast<int>(nx * static_cast<float>(tr.size.x));
		int oy = static_cast<int>(ny * static_cast<float>(tr.size.y));
		if (ox >= tr.size.x) {
			ox = tr.size.x - 1;
		}
		if (oy >= tr.size.y) {
			oy = tr.size.y - 1;
		}
		if (ox < 0) {
			ox = 0;
		}
		if (oy < 0) {
			oy = 0;
		}

		const sf::Texture& tex = sprite.getTexture();
		const sf::Vector2u texSize = tex.getSize();
		if (texSize.x == 0 || texSize.y == 0) {
			return false;
		}

		const int px = tr.position.x + ox;
		const int py = tr.position.y + oy;
		if (px < 0 || py < 0 || px >= static_cast<int>(texSize.x) || py >= static_cast<int>(texSize.y)) {
			return false;
		}

		const sf::Image& image = TexturePixelsForHitTest(tex);
		const sf::Vector2u imageSize = image.getSize();
		if (imageSize.x == 0 || imageSize.y == 0) {
			return false;
		}

		const unsigned pux = static_cast<unsigned>(px);
		const unsigned puy = static_cast<unsigned>(py);
		if (pux >= imageSize.x || puy >= imageSize.y) {
			return false;
		}

		const sf::Color texel = image.getPixel({pux, puy});
		const sf::Color tint = sprite.getColor();
		const int combinedAlpha = static_cast<int>(texel.a) * static_cast<int>(tint.a) / 255;
		return combinedAlpha > 0;
	}
} // namespace

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

	float Project(const sf::Vector2f& a, const sf::Vector2f& b) {
		auto result = Dot(a, b);
		if (auto lengthB = Length(b); lengthB > std::numeric_limits<float>::epsilon()) {
			result /= Length(b);
		}
		return result;
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

	sf::FloatRect AxisAlignedBoundsAfterTransform(const sf::Transform& m, const sf::FloatRect& localRect) {
		const float l = localRect.position.x;
		const float t = localRect.position.y;
		const float r = l + localRect.size.x;
		const float b = t + localRect.size.y;
		const sf::Vector2f p0 = m.transformPoint({l, t});
		const sf::Vector2f p1 = m.transformPoint({r, t});
		const sf::Vector2f p2 = m.transformPoint({l, b});
		const sf::Vector2f p3 = m.transformPoint({r, b});
		const float minX = std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x));
		const float maxX = std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x));
		const float minY = std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y));
		const float maxY = std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y));
		return {{minX, minY}, {maxX - minX, maxY - minY}};
	}

	bool IsWorldPointInsideOfShapeByFan(const sf::Vector2f& worldPoint, const sf::Shape* shape,
	                                    const sf::Transform& nodeWorld) {
		if (!shape) {
			return false;
		}
		sf::Transform combined = nodeWorld;
		combined *= shape->getTransform();
		return pointInsideConvexFan(worldPoint, shape->getPointCount(), [&](std::size_t i) {
			return combined.transformPoint(shape->getPoint(i));
		});
	}

	bool IsWorldPointInsideOfShapeByFan(const sf::Vector2f& worldPoint, const sf::Shape* shape) {
		return IsWorldPointInsideOfShapeByFan(worldPoint, shape, sf::Transform{});
	}

	bool IsWorldPointInsideOfShape(const sf::Vector2f& worldPoint, const sf::Shape* shape,
	                               const sf::Transform& nodeWorld) {
		if (!shape) {
			return false;
		}
		if (const auto* circle = dynamic_cast<const sf::CircleShape*>(shape)) {
			sf::Transform combined = nodeWorld;
			combined *= circle->getTransform();
			const sf::Vector2f localCenter = circle->getGeometricCenter();
			const sf::Vector2f center = combined.transformPoint(localCenter);
			const float radius =
			    Length(combined.transformPoint(localCenter + sf::Vector2f{circle->getRadius(), 0.f}) - center);
			return Sq(worldPoint.x - center.x) + Sq(worldPoint.y - center.y) <= Sq(radius);
		}
		if (const auto* rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
			sf::Transform combined = nodeWorld;
			combined *= rect->getTransform();
			const sf::Vector2f local = combined.getInverse().transformPoint(worldPoint);
			const sf::FloatRect localBounds({0.f, 0.f}, rect->getSize());
			return localBounds.contains(local);
		}
		return IsWorldPointInsideOfShapeByFan(worldPoint, shape, nodeWorld);
	}

	bool IsWorldPointInsideOfShape(const sf::Vector2f& worldPoint, const sf::Shape* shape) {
		return IsWorldPointInsideOfShape(worldPoint, shape, sf::Transform{});
	}

	bool IsWorldPointInsideOfVisual(const sf::Vector2f& worldPoint, const Visual* visual,
	                                const sf::Transform& nodeWorld) {
		if (!visual) {
			return false;
		}
		if (const auto* shapeVisual = dynamic_cast<const ShapeVisualBase*>(visual)) {
			return IsWorldPointInsideOfShape(worldPoint, shapeVisual->GetBaseShape(), nodeWorld);
		}
		if (const auto* fps = dynamic_cast<const TextVisual*>(visual)) {
			if (const sf::Text* text = fps->GetText()) {
				sf::Transform full = nodeWorld;
				full *= text->getTransform();
				const sf::Vector2f local = full.getInverse().transformPoint(worldPoint);
				return text->getLocalBounds().contains(local);
			}
			return false;
		}
		if (const auto* spriteVis = dynamic_cast<const SpriteVisual*>(visual)) {
			if (const sf::Sprite* sprite = spriteVis->GetSprite()) {
				return IsWorldPointInsideSpriteConsideringAlpha(worldPoint, *sprite, nodeWorld);
			}
			return false;
		}
		return false;
	}

	bool IsWorldPointInsideOfVisual(const sf::Vector2f& worldPoint, const Visual* visual) {
		return IsWorldPointInsideOfVisual(worldPoint, visual, sf::Transform{});
	}

	bool IsWorldPointInsideOfBody(const sf::Vector2f& worldPoint, const PhysicsBodyBehaviour* body) {
		if (!body) {
			return false;
		}
		if (const sf::Shape* shape = body->GetShape()) {
			if (auto n = Verify(body->GetNode())) {
				return IsWorldPointInsideOfShape(worldPoint, shape, n->GetWorldTransform());
			}
		}
		return pointInsideConvexFan(worldPoint, body->GetPointCount(), [&](std::size_t i) {
			return body->GetPointWorldPos(i);
		});
	}

	bool IsPointInsideOfTriangle(const sf::Vector2f& p, const sf::Vector2f& t1, const sf::Vector2f& t2,
	                             const sf::Vector2f& t3) {
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

	sf::CircleShape CreateCircle(const sf::Vector2f& pos, float radius, sf::Color color) {
		sf::CircleShape circle;
		circle.setPosition(pos);
		circle.setRadius(radius);
		circle.setOrigin({radius, radius});
		circle.setFillColor(color);
		return circle;
	}

	void MaximizeWindow(const sf::RenderWindow& window) {
#ifdef _WIN32
		ShowWindow(static_cast<HWND>(window.getNativeHandle()), SW_MAXIMIZE);
#endif
	}

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2i& pixel) {
		return window.mapPixelToCoords(pixel);
	}

	sf::Vector2f MapWindowPixelToWorld(const sf::RenderWindow& window, const sf::Vector2f& pixel) {
		return MapWindowPixelToWorld(window, sf::Vector2i(pixel));
	}

	sf::Vector2i MapWorldToWindowPixel(const sf::RenderWindow& window, const sf::Vector2f& world) {
		return window.mapCoordsToPixel(world);
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

	sf::Vector2f GetWorldPos(const std::shared_ptr<const SceneNode>& node) {
		const auto n = Verify(node);
		return n->GetWorldTransform().transformPoint(sf::Vector2f{});
	}

	void SetWorldPos(const std::shared_ptr<SceneNode>& node, sf::Vector2f pos) {
		const auto n = Verify(node);
		if (auto parent = n->GetParent()) {
			const sf::Vector2f local = parent->GetWorldTransform().getInverse().transformPoint(pos);
			n->GetLocalTransform()->SetPosition(local);
		}
		else {
			n->GetLocalTransform()->SetPosition(pos);
		}
	}

} // namespace Utils
