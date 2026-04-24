#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <SFML/System/Time.hpp>

class Behaviour : public EntityOnNode
{
public:
	~Behaviour() override = default;

	virtual void OnAttached() {}

	virtual void OnUpdate(const sf::Time& /*dt*/) {}
};
