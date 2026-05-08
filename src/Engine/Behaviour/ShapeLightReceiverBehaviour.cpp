#include "ShapeLightReceiverBehaviour.h"

#include "ShapeLightReceiverBehaviour.generated.hpp"

bool ShapeLightReceiverBehaviour::IsReceiverLightingEnabled() const {
	return _receiverLightingEnabled;
}

void ShapeLightReceiverBehaviour::SetReceiverLightingEnabled(bool value) {
	_receiverLightingEnabled = value;
}

bool ShapeLightReceiverBehaviour::IsBevelEmbossMode() const {
	return _bevelEmboss;
}

void ShapeLightReceiverBehaviour::SetBevelEmbossMode(bool value) {
	_bevelEmboss = value;
}

float ShapeLightReceiverBehaviour::GetBevelWidth() const {
	return _bevelWidth;
}

void ShapeLightReceiverBehaviour::SetBevelWidth(float value) {
	_bevelWidth = value;
}

bool ShapeLightReceiverBehaviour::IsEaseInCirc() const {
	return _easeInCirc;
}

void ShapeLightReceiverBehaviour::SetEaseInCirc(bool value) {
	_easeInCirc = value;
}

float ShapeLightReceiverBehaviour::GetDiffusion() const {
	return _diffusion;
}

void ShapeLightReceiverBehaviour::SetDiffusion(float value) {
	_diffusion = value;
}

float ShapeLightReceiverBehaviour::GetLightingStrength() const {
	return _lightingStrength;
}

void ShapeLightReceiverBehaviour::SetLightingStrength(float value) {
	_lightingStrength = value;
}
