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
	/// @property(name="Influence radius (world)", dragSpeed=2.f, tooltip="Radius in world units; scales with camera zoom when applied.")
	float _influenceRadius = 120.f;
	/// @property(dragSpeed=0.0001f)
	sf::Vector2f _uvOffset{};
};
