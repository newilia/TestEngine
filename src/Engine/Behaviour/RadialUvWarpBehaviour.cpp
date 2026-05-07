#include "RadialUvWarpBehaviour.h"

#include "RadialUvWarpBehaviour.generated.hpp"

float RadialUvWarpBehaviour::GetWarpStrength() const {
	return _warpStrength;
}

void RadialUvWarpBehaviour::SetWarpStrength(float value) {
	_warpStrength = value;
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

bool RadialUvWarpBehaviour::IsRadialUvWarpEnabled() const {
	return _isRadialUvWarpEnabled;
}

void RadialUvWarpBehaviour::SetRadialUvWarpEnabled(bool value) {
	_isRadialUvWarpEnabled = value;
}
