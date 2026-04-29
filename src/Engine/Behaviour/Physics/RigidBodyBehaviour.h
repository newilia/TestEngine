#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

#include <limits>

class RigidBodyBehaviour : public Behaviour
{
	META_CLASS()
public:
	/// @property(tooltip="Infinity = immovable; use SetImmovable() in code, or set mass in inspector.")
	float _mass = 1.f;
	/// @property
	sf::Vector2f _velocity;
	/// @property(name="Angle (rad)")
	float _angle = 0.f;
	/// @property
	float _angularSpeed = 0.f;
	/// @property
	float _restitution = 0.5f;
	/// @property
	float _friction = 0.5f;

	void SetImmovable() { _mass = std::numeric_limits<float>::infinity(); }

	bool IsImmovable() const { return _mass == std::numeric_limits<float>::infinity(); }
};
