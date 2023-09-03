#include "VectorArrow.h"
#include "Utils.h"

void VectorArrow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	sf::Vertex vertices[2];
	vertices[0].position = mStart;
	vertices[1].position = mEnd;
	vertices[0].color = mColor;
	vertices[1].color = mColor;

	target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);

	if (mArrowHeadEnabled) {
		sf::Vector2f v = mStart - mEnd;
		for (float i = -0.5f; i < 0.51f; ++i) {
			auto ahVector = utils::rotate(v, mArrowHeadAngle * i);
			ahVector = utils::normalize(ahVector) * mArrowHeadSize;
			vertices[0] = mEnd;
			vertices[1] = mEnd + ahVector;
			vertices[0].color = mColor;
			vertices[1].color = mColor;
			target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);
		}		
	}
}
