#include "VectorArrowVisual.h"

#include <SFML/Graphics/RenderTarget.hpp>

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

void VectorArrowVisual::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_visible) {
		return;
	}
	target.draw(_arrow, states);
}

bool VectorArrowVisual::HitTest(sf::Vector2f /*windowPosition*/) const {
	return false;
}
