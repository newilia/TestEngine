#include "Engine/Visual/VectorArrowVisual.h"

#include "Engine/Core/Utils.h"
#include "VectorArrowVisual.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

void VectorArrowVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(_arrowShape, states);
}

bool VectorArrowVisual::HitTest(const sf::Vector2f& /*worldPoint*/) const {
	return false;
}

const sf::Shape* VectorArrowVisual::GetBaseShape() const {
	return &_arrowShape;
}

sf::FloatRect VectorArrowVisual::GetLocalBounds() const {
	return _arrowShape.GetLocalBounds();
}

sf::Vector2f VectorArrowVisual::GetStartPos() const {
	return _arrowShape.GetStartPos();
}

sf::Vector2f VectorArrowVisual::GetEndPos() const {
	return _arrowShape.GetEndPos();
}

sf::Color VectorArrowVisual::GetFillColor() const {
	return _arrowShape.getFillColor();
}

void VectorArrowVisual::SetStartPos(const sf::Vector2f& start) {
	_arrowShape.SetStartPos(start);
}

void VectorArrowVisual::SetEndPos(const sf::Vector2f& end) {
	_arrowShape.SetEndPos(end);
}

void VectorArrowVisual::SetFillColor(const sf::Color& color) {
	_arrowShape.setFillColor(color);
}

sf::Angle VectorArrowVisual::GetArrowHeadAngle() const {
	return _arrowShape.GetArrowHeadAngle();
}

float VectorArrowVisual::GetArrowHeadSize() const {
	return _arrowShape.GetArrowHeadSize();
}

void VectorArrowVisual::SetArrowHeadAngle(sf::Angle angle) {
	_arrowShape.SetArrowHeadAngle(angle);
}

void VectorArrowVisual::SetArrowHeadSize(float size) {
	_arrowShape.SetArrowHeadSize(size);
}
