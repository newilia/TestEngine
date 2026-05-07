#include "GravitationalLensBehaviour.h"

#include "GravitationalLensBehaviour.generated.hpp"

float GravitationalLensBehaviour::GetAmplitude() const {
	return _amplitude;
}

void GravitationalLensBehaviour::SetAmplitude(float value) {
	_amplitude = value;
}

float GravitationalLensBehaviour::GetFalloff() const {
	return _falloff;
}

void GravitationalLensBehaviour::SetFalloff(float value) {
	_falloff = value;
}

const sf::Vector2f& GravitationalLensBehaviour::GetUvOffset() const {
	return _uvOffset;
}

void GravitationalLensBehaviour::SetUvOffset(const sf::Vector2f& value) {
	_uvOffset = value;
}

bool GravitationalLensBehaviour::IsLensEffectEnabled() const {
	return _isLensEffectEnabled;
}

void GravitationalLensBehaviour::SetLensEffectEnabled(bool value) {
	_isLensEffectEnabled = value;
}
