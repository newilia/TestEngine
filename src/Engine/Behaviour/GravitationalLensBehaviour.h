#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

class GravitationalLensBehaviour : public Behaviour
{
	META_CLASS()

public:
	float GetAmplitude() const;
	void SetAmplitude(float value);

	float GetFalloff() const;
	void SetFalloff(float value);

	const sf::Vector2f& GetUvOffset() const;
	void SetUvOffset(const sf::Vector2f& value);

	bool IsLensEffectEnabled() const;
	void SetLensEffectEnabled(bool value);

private:
	/// @property
	bool _isLensEffectEnabled = true;
	/// @property(dragSpeed=0.0001f)
	float _amplitude = 0.035f;
	/// @property(dragSpeed=0.0001f)
	float _falloff = 0.0012f;
	/// @property(dragSpeed=0.0001f)
	sf::Vector2f _uvOffset{};
};
