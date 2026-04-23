#pragma once
#include "SFML/Graphics.hpp"

class Updateable
{
public:
	virtual ~Updateable() {}

	virtual void Update(const sf::Time& dt) = 0;
};