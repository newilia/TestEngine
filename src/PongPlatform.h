#pragma once
#include "PlatformControllerBase.h"
#include "ShapeBody.h"
#include "SFML/Graphics.hpp"

class PongPlatform : public ShapeBody<sf::ConvexShape> {
public:
    PongPlatform();
    void init() override;
    void setShapeDimensions(float width, float height, float rotationDeg);
    void update(const sf::Time& dt) override;
    shared_ptr<PlatformControllerBase> getController() const { return mController; }
    void setController(const shared_ptr<PlatformControllerBase>& controller) { mController = controller; }
private:
    shared_ptr<PlatformControllerBase> mController;
};