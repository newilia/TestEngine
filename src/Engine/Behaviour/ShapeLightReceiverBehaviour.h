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

	bool IsEaseOutCirc() const;
	void SetEaseOutCirc(bool value);

	float GetDiffusion() const;
	void SetDiffusion(float value);

private:
	/// @property(name="Lighting enabled")
	bool _receiverLightingEnabled = true;
	/// @property(name="Bevel emboss", tooltip="Off = radial gradient from lights; On = edge bevel / emboss")
	bool _bevelEmboss = false;
	/// @property(dragSpeed=0.5f, minValue=0.01f)
	float _bevelWidth = 14.f;
	/// @property(name="Ease out circ", tooltip="Off = linear falloff curve")
	bool _easeOutCirc = true;
	/// @property(minValue=0.f, maxValue=1.f, dragSpeed=0.01f)
	float _diffusion = 0.45f;
};
