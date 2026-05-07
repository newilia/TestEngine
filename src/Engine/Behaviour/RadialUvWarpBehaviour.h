#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

class RadialUvWarpBehaviour : public Behaviour
{
	META_CLASS()

public:
	float GetWarpStrength() const;
	void SetWarpStrength(float value);

	float GetInfluenceRadius() const;
	void SetInfluenceRadius(float value);

	const sf::Vector2f& GetUvOffset() const;
	void SetUvOffset(const sf::Vector2f& value);

	bool IsRadialUvWarpEnabled() const;
	void SetRadialUvWarpEnabled(bool value);

private:
	/// @property(name="Enabled")
	bool _isRadialUvWarpEnabled = true;
	/// @property(name="Radial warp strength", dragSpeed=0.01f, tooltip="Positive: magnify around the center. Negative: compress.")
	float _warpStrength = 0.35f;
	/// @property(name="Influence radius", dragSpeed=0.001f, tooltip="Aspect-correct radius of the warp in normalized screen space (~0–1).")
	float _influenceRadius = 0.12f;
	/// @property(dragSpeed=0.0001f)
	sf::Vector2f _uvOffset{};
};
