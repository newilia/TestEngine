#pragma once
#include "ShapeBody.h"
#include "UserPlatformContoller.h"
#include "SFML/Graphics.hpp"

class PongPlatform : public ShapeBody<sf::ConvexShape> {
public:
    PongPlatform();
    void init() override;
    void initShape(float width, float height, float angleDeg);
    void update(const sf::Time& dt) override;
    UserPlatformController& getController() { return mController; }
private:
    UserPlatformController mController;
};