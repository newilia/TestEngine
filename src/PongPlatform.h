#pragma once
#include "Updateable.h"
#include "SFML/Graphics.hpp"

class PongPlatform : public Updateable {
public:
    PongPlatform();
    void update(const sf::Time& dt) override;

    sf::RectangleShape& getShape() { return mShape; }
    void setAirFriction(float friction) { mAirFriction = friction; }
    void setMaxSpeed(float maxSpeed) { mMaxSpeed = maxSpeed; }
    void setMoveArea(const sf::FloatRect& moveArea) { mMoveArea = moveArea; }
    void setPlayerAccelMagnitude(float val) { mPlayerAccelMagnitude = val; }
    void handleInput(const sf::Event& event);

protected:
    float mMaxSpeed = std::numeric_limits<float>::max();
    float mAirFriction = 0.f;
    float mPlayerAccelMagnitude = 1000.f;
    sf::Vector2f mPlayerAccel;
    sf::Vector2f mSpeed;
    sf::FloatRect mMoveArea;
    sf::RectangleShape mShape;
};