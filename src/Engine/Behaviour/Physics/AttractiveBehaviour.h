#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <memory>

class AttractiveBehaviour : public Behaviour, public std::enable_shared_from_this<AttractiveBehaviour>
{
	META_CLASS()

public:
	void OnInit() override;
	void OnDeinit() override;

	bool IsEnabled() const;
	void SetEnabled(bool isEnabled);

	float GetAttraction() const;
	void SetAttraction(float value);

	float GetFalloffExponent() const;
	void SetFalloffExponent(float value);

private:
	/// @property
	bool _isEnabled = true;
	/// @property
	float _attraction = 0.f;
	/// @property(minValue=0.f, maxValue=4.f, dragSpeed=0.01f)
	float _falloffExponent = 2.f;
};
