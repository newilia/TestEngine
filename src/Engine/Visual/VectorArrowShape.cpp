#include "VectorArrowShape.h"

#include "Engine/Core/Utils.h"

VectorArrowShape::VectorArrowShape(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& fillColor)
    : _start(start), _end(end) {
	setFillColor(fillColor);
	UpdateVertices();
}

void VectorArrowShape::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(_vertices.data(), _vertices.size(), sf::PrimitiveType::LineStrip, states);
}

void VectorArrowShape::UpdateVertices() {
	auto color = getFillColor();
	_vertices[0] = {_start, color};
	_vertices[1] = {_end, color};
	{
		sf::Vector2f vec = _start - _end;
		for (int i = 0; i < 2; i++) {
			auto headVertex = Utils::Rotate(vec, _arrowHeadAngle.asRadians() * (-1 + i * 2));
			headVertex = Utils::Normalize(headVertex) * _arrowHeadSize;
			_vertices[2 + i] = {_end + headVertex, color};
		}
	}
	_vertices[4] = {_end, color};
	update();
}

sf::Vector2f VectorArrowShape::getPoint(std::size_t index) const {
	return _vertices[index].position;
}

std::size_t VectorArrowShape::getPointCount() const {
	return _vertices.size();
}

sf::FloatRect VectorArrowShape::GetLocalBounds() const {
	constexpr float padding = 5.f;
	auto left = std::min(_start.x, _end.x) - padding;
	auto top = std::min(_start.y, _end.y) - padding;
	auto right = std::max(_start.x, _end.x) + padding;
	auto bottom = std::max(_start.y, _end.y) + padding;
	auto width = right - left;
	auto height = bottom - top;
	return {{left, top}, {width, height}};
}

void VectorArrowShape::SetStartPos(const sf::Vector2f& start) {
	_start = start;
	UpdateVertices();
}

void VectorArrowShape::SetEndPos(const sf::Vector2f& end) {
	_end = end;
	UpdateVertices();
}

void VectorArrowShape::SetArrowHeadAngle(sf::Angle angle) {
	_arrowHeadAngle = angle;
	UpdateVertices();
}

void VectorArrowShape::SetArrowHeadSize(float size) {
	_arrowHeadSize = size;
	UpdateVertices();
}

sf::Vector2f VectorArrowShape::GetStartPos() const {
	return _start;
}

sf::Vector2f VectorArrowShape::GetEndPos() const {
	return _end;
}

sf::Angle VectorArrowShape::GetArrowHeadAngle() const {
	return _arrowHeadAngle;
}

float VectorArrowShape::GetArrowHeadSize() const {
	return _arrowHeadSize;
}
