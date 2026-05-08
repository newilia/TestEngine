#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Color.hpp>

class PointLightBehaviour : public Behaviour
{
	META_CLASS()

public:
	sf::Color GetLightColor() const;
	void SetLightColor(const sf::Color& color);

	float GetIntensity() const;
	void SetIntensity(float value);

	float GetRadius() const;
	void SetRadius(float value);

	bool IsLightEnabled() const;
	void SetLightEnabled(bool value);

private:
	/// @property
	sf::Color _lightColor = sf::Color::White;
	/// @property(dragSpeed=0.02f, minValue=0.f)
	float _intensity = 1.f;
	/// @property(dragSpeed=2.f, minValue=1.f)
	float _radius = 400.f;
	/// @property(name="Enabled")
	bool _lightEnabled = true;
};
