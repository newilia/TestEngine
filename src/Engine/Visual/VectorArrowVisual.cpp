#include "Engine/Visual/VectorArrowVisual.h"

#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RenderTarget.hpp>

VectorArrow::VectorArrow() = default;

VectorArrow::VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color)
    : _start(start), _end(end), _color(color) {}

VectorArrow::~VectorArrow() = default;

void VectorArrow::SetStartPos(const sf::Vector2f& start) {
	_start = start;
}

void VectorArrow::SetEndPos(const sf::Vector2f& end) {
	_end = end;
}

void VectorArrow::SetColor(const sf::Color& color) {
	_color = color;
}

void VectorArrow::EnableArrowHead(bool enable) {
	_arrowHeadEnabled = enable;
}

void VectorArrow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_start == _end) {
		return;
	}
	sf::Vertex vertices[2];
	vertices[0].position = _start;
	vertices[1].position = _end;
	vertices[0].color = _color;
	vertices[1].color = _color;

	target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);

	if (_arrowHeadEnabled) {
		sf::Vector2f v = _start - _end;
		for (float i = -0.5f; i < 0.51f; ++i) {
			auto ahVector = Utils::Rotate(v, _arrowHeadAngle * i);
			ahVector = Utils::Normalize(ahVector) * _arrowHeadSize;
			vertices[0].position = _end;
			vertices[1].position = _end + ahVector;
			vertices[0].color = _color;
			vertices[1].color = _color;
			target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);
		}
	}
}

void VectorArrowVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_visible) {
		return;
	}
	target.draw(_arrow, states);
}

bool VectorArrowVisual::HitTest(const sf::Vector2f& /*worldPoint*/) const {
	return false;
}

void VectorArrowVisual::SetStartPos(const sf::Vector2f& start) {
	_arrow.SetStartPos(start);
}

void VectorArrowVisual::SetEndPos(const sf::Vector2f& end) {
	_arrow.SetEndPos(end);
}

void VectorArrowVisual::SetColor(const sf::Color& color) {
	_arrow.SetColor(color);
}

void VectorArrowVisual::SetVisible(bool visible) {
	_visible = visible;
}
