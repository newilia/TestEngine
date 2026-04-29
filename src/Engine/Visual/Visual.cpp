#include "Engine/Visual/Visual.h"

void Visual::OnTap(sf::Vector2f windowPosition) {
	(void)windowPosition;
}

void Visual::SetTapHandlingEnabled(bool enabled) {
	_tapHandlingEnabled = enabled;
}

bool Visual::IsTapHandlingEnabled() const {
	return _tapHandlingEnabled;
}

void Visual::SetTransparentToTap(bool transparent) {
	_transparentToTap = transparent;
}

bool Visual::IsTransparentToTap() const {
	return _transparentToTap;
}
