#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/System/Vector2.hpp>

class UserPullBehaviour : public Behaviour
{
	META_CLASS()
public:
	void OnAttached() override {}

	~UserPullBehaviour() override = default;

	sf::Vector2f GetPullVector() const;

	/// @property
	sf::Vector2f _localSourcePoint;
	/// @property
	sf::Vector2f _globalDestPoint;
	/// @property
	float _pullingStrength = 100000.f;

	enum class PullMode
	{
		POSITION,
		FORCE,
		VELOCITY,
	} _mode = PullMode::FORCE;
};
