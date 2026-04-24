#pragma once

#include <SFML/System/Time.hpp>

class Updatable
{
public:
	virtual ~Updatable() {}

	virtual void Update(const sf::Time& dt) = 0;
};
