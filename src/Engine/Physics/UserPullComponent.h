#pragma once
#include "Engine/ComponentBase.h"

#include <SFML/System/Vector2.hpp>

class UserPullComponent : public ComponentBase
{
public:
	UserPullComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	~UserPullComponent() override = default;

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
