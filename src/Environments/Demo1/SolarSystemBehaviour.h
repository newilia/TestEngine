#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <memory>

class AttractiveBehaviour;
class SceneNode;

/// Demo1 solar system: spawns bodies under `SolarSystemRoot` and exposes inspector-tunable simulation knobs.
class SolarSystemBehaviour : public Behaviour
{
	META_CLASS()

public:
	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;

	/// @method
	void Restart();
	void SetSolarSystemRoot(std::weak_ptr<SceneNode> root);

private:
	void SyncSunAttractionFromProperty() const;

	std::weak_ptr<SceneNode> _solarSystemRoot;
	std::weak_ptr<AttractiveBehaviour> _sunAttractive;

	/// @property(dragSpeed=0.1f, tooltip="Passed to the Sun AttractiveBehaviour (negative = pull)")
	float _sunAttraction = -260.f;
	/// @property(dragSpeed=0.5f, tooltip="AU to world units (pixels)")
	float _distanceScale = 100.f;
	/// @property(dragSpeed=0.1f, tooltip="Multiplier for planet circle radii vs Earth-normalized model")
	float _visualRadiusScale = 1.f;
	/// @property
	float _outlineThickness = 2.f;
	/// @property(minValue=1e-12f, maxValue=1e6f, dragSpeed=0.01f, tooltip="Scales planet PhysicsBody masses vs Earth=1 model")
	float _massScale = 0.1f;
	/// @property(minValue=0.f, maxValue=100.f, dragSpeed=0.05f, tooltip="Scales initial tangential speed vs mean km/s model")
	float _orbitalSpeedScale = 3.f;
};
