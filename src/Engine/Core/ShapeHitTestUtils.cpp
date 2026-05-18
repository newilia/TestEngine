#include "Engine/Core/ShapeHitTestUtils.h"

#include "Engine/Core/MathUtils.h"
#include "Engine/Visual/ShapeVisualBase.h"
#include "Engine/Visual/SpriteVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "Engine/Visual/Visual.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <limits>
#include <unordered_map>

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

	bool IsWorldPointInsideSpriteConsideringAlpha(
	    const sf::Vector2f& worldPoint, const sf::Sprite& sprite, const sf::Transform& nodeWorld) {
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
	bool IsWorldPointInsideOfShapeByFan(
	    const sf::Vector2f& worldPoint, const sf::Shape* shape, const sf::Transform& nodeWorld) {
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

	bool IsWorldPointInsideOfShape(
	    const sf::Vector2f& worldPoint, const sf::Shape* shape, const sf::Transform& nodeWorld) {
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

	bool IsWorldPointInsideOfVisual(
	    const sf::Vector2f& worldPoint, const Visual* visual, const sf::Transform& nodeWorld) {
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
} // namespace Utils
