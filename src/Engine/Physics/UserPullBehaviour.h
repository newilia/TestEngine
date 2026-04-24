#pragma once

#include "Engine/Core/Behaviour.h"

#include <SFML/System/Vector2.hpp>

class UserPullBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	~UserPullBehaviour() override = default;

	sf::Vector2f getPullVector() const;

	sf::Vector2f _localSourcePoint;
	sf::Vector2f _globalDestPoint;
	float _pullingStrength = 100000.f;

	enum class PullMode
	{
		POSITION,
		FORCE,
		VELOCITY,
	} _mode = PullMode::FORCE;
};
