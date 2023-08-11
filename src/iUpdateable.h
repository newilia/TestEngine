#pragma once
#include "SFML/Graphics.hpp"

class iUpdateable {
public:
	virtual void update(const sf::Time& dt) = 0;
};