#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Color.hpp>

#include <memory>

class PointLightBehaviour : public Behaviour, public std::enable_shared_from_this<PointLightBehaviour>
{
	META_CLASS()

public:
	void OnInit() override;
	void OnDeinit() override;
	void OnEnabled(bool isEnabled) override;

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
	bool _lightEnabled = true;
	/// @property
	sf::Color _lightColor = sf::Color::White;
	/// @property(minValue=0.f)
	float _intensity = 1.f;
	/// @property(minValue=1.f)
	float _radius = 400.f;
};
