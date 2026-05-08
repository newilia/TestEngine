#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

class ShapeLightReceiverBehaviour : public Behaviour
{
	META_CLASS()

public:
	bool IsReceiverLightingEnabled() const;
	void SetReceiverLightingEnabled(bool value);

	bool IsBevelEmbossMode() const;
	void SetBevelEmbossMode(bool value);

	float GetBevelWidth() const;
	void SetBevelWidth(float value);

	bool IsEaseInCirc() const;
	void SetEaseInCirc(bool value);

	float GetDiffusion() const;
	void SetDiffusion(float value);

	float GetLightingStrength() const;
	void SetLightingStrength(float value);

private:
	/// @property(name="Lighting enabled")
	bool _receiverLightingEnabled = true;
	/// @property(name="Bevel emboss", tooltip="Off = radial gradient from lights; On = edge bevel / emboss")
	bool _bevelEmboss = false;
	/// @property(dragSpeed=0.5f, minValue=0.01f)
	float _bevelWidth = 14.f;
	/// @property(name="Ease in circ", tooltip="Off = linear falloff curve")
	bool _easeInCirc = true;
	/// @property(minValue=0.f, maxValue=1.f, dragSpeed=0.01f)
	float _diffusion = 1.f;
	/// @property(name="Lighting strength", tooltip="How strongly all point lights add to this shape (0 = none, 1 = full)", minValue=0.f, maxValue=1.f, dragSpeed=0.01f)
	float _lightingStrength = 1.f;
};
