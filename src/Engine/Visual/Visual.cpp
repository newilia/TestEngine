#include "Engine/Visual/Visual.h"

#include "Visual.generated.hpp"

void Visual::OnTap(const sf::Vector2f& /*worldPoint*/) {}

void Visual::SetTapHandlingEnabled(bool enabled) {
	_tapHandlingEnabled = enabled;
}

bool Visual::IsTapHandlingEnabled() const {
	return _tapHandlingEnabled;
}
