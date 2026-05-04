#include "Engine/Visual/VectorArrowVisual.h"

#include "Engine/Core/Utils.h"
#include "VectorArrowVisual.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

void VectorArrowVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	target.draw(_arrow, states);
}

bool VectorArrowVisual::HitTest(const sf::Vector2f& /*worldPoint*/) const {
	return false;
}

sf::Vector2f VectorArrowVisual::GetStartPos() const {
	return _arrow.GetStartPos();
}

sf::Vector2f VectorArrowVisual::GetEndPos() const {
	return _arrow.GetEndPos();
}

sf::Color VectorArrowVisual::GetColor() const {
	return _arrow.GetColor();
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

float VectorArrowVisual::GetArrowHeadAngle() const {
	return _arrow.GetArrowHeadAngle();
}

float VectorArrowVisual::GetArrowHeadSize() const {
	return _arrow.GetArrowHeadSize();
}

void VectorArrowVisual::SetArrowHeadAngle(float angle) {
	_arrow.SetArrowHeadAngle(angle);
}

void VectorArrowVisual::SetArrowHeadSize(float size) {
	_arrow.SetArrowHeadSize(size);
}
