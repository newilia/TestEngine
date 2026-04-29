#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

class UserPullBehaviour : public Behaviour
{
	META_CLASS()
public:
	enum class PullMode
	{
		POSITION,
		FORCE,
		VELOCITY,
	};
	~UserPullBehaviour() override = default;
	sf::Vector2f GetPullVector() const;
	/// @property
	sf::Vector2f _localSourcePoint;
	/// @property
	sf::Vector2f _globalDestPoint;
	/// @property
	float _pullingStrength = 100000.f;
	PullMode _mode = PullMode::FORCE;
};
