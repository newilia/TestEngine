#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

class RadialUvWarpBehaviour : public Behaviour
{
	META_CLASS()

public:
	bool IsEnabled() const;
	void SetEnabled(bool value);

	float GetIntensity() const;
	void SetIntensity(float value);

	float GetInfluenceRadius() const;
	void SetInfluenceRadius(float value);

	const sf::Vector2f& GetUvOffset() const;
	void SetUvOffset(const sf::Vector2f& value);

private:
	/// @property
	bool _isEnabled = true;
	/// @property(dragSpeed=0.01f)
	float intensity = 0.35f;
	/// @property(dragSpeed=2.f, minValue=0.f)
	float _influenceRadius = 120.f;
	/// @property(dragSpeed=0.0001f)
	sf::Vector2f _uvOffset{};
};
