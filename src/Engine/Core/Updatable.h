#pragma once

#include <SFML/System/Time.hpp>

class Updatable
{
public:
	virtual ~Updatable() {}

	virtual void Run(const sf::Time& dt) = 0;
};
