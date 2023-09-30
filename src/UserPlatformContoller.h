#pragma once
#include <SFML/System/Vector2.hpp>

#include "PlatformControllerBase.h"
#include "Updateable.h"

class PongPlatform;

namespace sf {
	class Event;
}

class UserPlatformController : public PlatformControllerBase {
public:
	UserPlatformController(PongPlatform* platform) : PlatformControllerBase(platform) {};
	void init() override;
	void handleEvent(const sf::Event& event);
	void setVerticalFreedomFactor(float val) { mVerticalMoveFactor = val; }
	void setVelocityFactor(float val) { mSpeedFactor = val; }
	void setVelLimit(sf::Vector2f val) { mVelLimit = val; }
	void update(const sf::Time& dt) override;

protected:
	sf::Vector2f mDefaultPos;
	sf::Vector2f mTargetPos;
	sf::Vector2f mLastPos;
	float mVerticalMoveFactor = 0.9f;
	float mSpeedFactor = 50.f;
	sf::Vector2f mVelLimit;
};
