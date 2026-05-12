#include "RadialUvWarpBehaviour.h"

#include "RadialUvWarpBehaviour.generated.hpp"

bool RadialUvWarpBehaviour::IsEnabled() const {
	return _isEnabled;
}

void RadialUvWarpBehaviour::SetEnabled(bool value) {
	_isEnabled = value;
}

float RadialUvWarpBehaviour::GetIntensity() const {
	return intensity;
}

void RadialUvWarpBehaviour::SetIntensity(float value) {
	intensity = value;
}

float RadialUvWarpBehaviour::GetInfluenceRadius() const {
	return _influenceRadius;
}

void RadialUvWarpBehaviour::SetInfluenceRadius(float value) {
	_influenceRadius = value;
}

const sf::Vector2f& RadialUvWarpBehaviour::GetUvOffset() const {
	return _uvOffset;
}

void RadialUvWarpBehaviour::SetUvOffset(const sf::Vector2f& value) {
	_uvOffset = value;
}
