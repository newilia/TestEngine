#pragma once
#include "SFML/Graphics.hpp"

class Updateable {
public:
	virtual ~Updateable() {}
	virtual void update(const sf::Time& dt) = 0;
};