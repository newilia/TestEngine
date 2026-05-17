#include "Engine/Visual/CircleShapeVisual.h"

#include "CircleShapeVisual.generated.hpp"
#include "Engine/Core/Utils.h"

#include <algorithm>
#include <cstddef>

namespace {
	constexpr int kMinCirclePoints = 3;
	constexpr int kDefaultCirclePoints = 64;
	constexpr int kMaxCirclePoints = 512;
} // namespace

CircleShapeVisual::CircleShapeVisual() {
	_circle.setPointCount(kDefaultCirclePoints);
}

const sf::Shape* CircleShapeVisual::GetBaseShape() const {
	return &_circle;
}

float CircleShapeVisual::GetRadius() const {
	return _circle.getRadius();
}

void CircleShapeVisual::SetRadius(float radius) {
	_circle.setRadius(radius);
	_circle.setOrigin({radius, radius});
	_isSectorDirty = true;
}

void CircleShapeVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	ShapeVisualBase::Draw(target, states);

	if (_drawSector) {
		if (_isSectorDirty) {
			RebuildSectorVertices();
		}
		sf::RenderStates sectorStates = states;
		sectorStates.transform.translate(_circle.getPosition());
		target.draw(_sectorVertices, sectorStates);
	}
}

void CircleShapeVisual::RebuildSectorVertices() const {
	constexpr int kSectorPoints = 10;
	constexpr sf::Angle kAngleStep = sf::degrees(30) / (kSectorPoints - 1);
	const float radius = GetRadius();

	_sectorVertices.setPrimitiveType(sf::PrimitiveType::TriangleFan);
	_sectorVertices.resize(kSectorPoints + 2);
	_sectorVertices[0].position = {0.f, 0.f};
	_sectorVertices[0].color = _sectorColor;
	_sectorVertices[kSectorPoints + 1].position = {0.f, 0.f};
	_sectorVertices[kSectorPoints + 1].color = _sectorColor;

	for (int i = 0; i < kSectorPoints; i++) {
		const sf::Angle angle = kAngleStep * i;
		_sectorVertices[i + 1].position = Utils::Rotate(sf::Vector2f(0, radius), angle.asRadians());
		_sectorVertices[i + 1].color = _sectorColor;
	}
	_isSectorDirty = false;
}

int CircleShapeVisual::GetPointCount() const {
	return static_cast<int>(_circle.getPointCount());
}

void CircleShapeVisual::SetPointCount(int count) {
	const int clamped = std::clamp(count, kMinCirclePoints, kMaxCirclePoints);
	_circle.setPointCount(static_cast<std::size_t>(clamped));
}

bool CircleShapeVisual::GetDrawSector() const {
	return _drawSector;
}

void CircleShapeVisual::SetDrawSector(bool drawSector) {
	_drawSector = drawSector;
}

sf::Color CircleShapeVisual::GetSectorColor() const {
	return _sectorColor;
}

void CircleShapeVisual::SetSectorColor(sf::Color color) {
	_sectorColor = color;
	_isSectorDirty = true;
}
