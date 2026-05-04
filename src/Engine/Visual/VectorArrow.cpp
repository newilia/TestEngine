#include "VectorArrow.h"

#include "Engine/Core/Utils.h"

VectorArrow::VectorArrow(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color)
    : _start(start), _end(end), _color(color) {}

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

void VectorArrow::SetStartPos(const sf::Vector2f& start) {
	_start = start;
}

void VectorArrow::SetEndPos(const sf::Vector2f& end) {
	_end = end;
}

void VectorArrow::SetColor(const sf::Color& color) {
	_color = color;
}

void VectorArrow::SetArrowHeadAngle(float angle) {
	_arrowHeadAngle = angle;
}

void VectorArrow::SetArrowHeadSize(float size) {
	_arrowHeadSize = size;
}

sf::Vector2f VectorArrow::GetStartPos() const {
	return _start;
}

sf::Vector2f VectorArrow::GetEndPos() const {
	return _end;
}

sf::Color VectorArrow::GetColor() const {
	return _color;
}

float VectorArrow::GetArrowHeadAngle() const {
	return _arrowHeadAngle;
}

float VectorArrow::GetArrowHeadSize() const {
	return _arrowHeadSize;
}
