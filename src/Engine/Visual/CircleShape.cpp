#include "Engine/Visual/CircleShape.h"

#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <SFML/System/Angle.hpp>

namespace {

	constexpr int kDefaultCirclePoints = 64;

} // namespace

CircleShape::CircleShape() {
	setPointCount(kDefaultCirclePoints);
}

void CircleShape::setRadius(float radius) {
	sf::CircleShape::setRadius(radius);

	setOrigin({radius, radius});

	_isSectorDirty = true;
}

bool CircleShape::GetDrawSector() const {
	return _drawSector;
}

void CircleShape::SetDrawSector(bool drawSector) {
	_drawSector = drawSector;
}

sf::Color CircleShape::GetSectorColor() const {
	return _sectorColor;
}

void CircleShape::SetSectorColor(sf::Color color) {
	_sectorColor = color;

	_isSectorDirty = true;
}

void CircleShape::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	const sf::CircleShape body = static_cast<const sf::CircleShape&>(*this);

	target.draw(body, states);

	if (_drawSector) {
		DrawSector(target, states);
	}
}

void CircleShape::DrawSector(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_isSectorDirty) {
		RebuildSectorVertices();
	}

	sf::RenderStates sectorStates = states;

	sectorStates.transform.translate(getPosition());

	target.draw(_sectorVertices, sectorStates);
}

void CircleShape::RebuildSectorVertices() const {
	constexpr int kSectorPoints = 10;

	constexpr sf::Angle kAngleStep = sf::degrees(30) / (kSectorPoints - 1);

	const float radius = getRadius();

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
