#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/RenderTarget.hpp>

void ShapeVisualBase::Draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (GetShape()) {
		target.draw(*GetShape(), states);
	}
}
