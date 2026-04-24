#pragma once

#include "Engine/Behaviour/Behaviour.h"

#include <SFML/System/Vector2.hpp>

#include <limits>

class RigidBodyBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	float _mass = 1.f;
	sf::Vector2f _velocity;

	float _angle = 0.f;
	float _angularSpeed = 0.f;

	float _restitution = 0.5f;
	float _friction = 0.5f;

	void SetImmovable() { _mass = std::numeric_limits<float>::infinity(); }

	bool IsImmovable() const { return _mass == std::numeric_limits<float>::infinity(); }
};
