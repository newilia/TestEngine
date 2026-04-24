#include "VectorArrow.h"

#include "Engine/App/Utils.h"

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
			auto ahVector = utils::rotate(v, _arrowHeadAngle * i);
			ahVector = utils::normalize(ahVector) * _arrowHeadSize;
			vertices[0].position = _end;
			vertices[1].position = _end + ahVector;
			vertices[0].color = _color;
			vertices[1].color = _color;
			target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);
		}
	}
}
