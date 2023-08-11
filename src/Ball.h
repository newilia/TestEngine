#pragma once
#include "Ball.h"
#include "iUpdateable.h"
#include "SFML/Graphics.hpp"

class Ball : public iUpdateable {
public:
	Ball() = default;
	void update(const sf::Time& dt) override;
	sf::Vector2f getSpeed() const { return mSpeed; }
	void setSpeed(const sf::Vector2f& speed) { mSpeed = speed; }
	void setMoveArea(const sf::FloatRect& moveArea) { mMoveArea = moveArea; }
	void setRadius(float r);
	sf::CircleShape& getShape() { return mShape; }

private:
	sf::CircleShape mShape;
	sf::Vector2f mSpeed;
	sf::FloatRect mMoveArea;
};