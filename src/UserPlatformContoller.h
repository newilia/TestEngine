#pragma once
#include <SFML/System/Vector2.hpp>

#include "Updateable.h"

class PongPlatform;

namespace sf {
	class Event;
}

class UserPlatformController : public Updateable {
public:
	UserPlatformController(PongPlatform* platform);
	void init();
	void handleEvent(const sf::Event& event);
	void setVerticalMoveFactor(float val) { mVerticalMoveFactor = val; }
	void setSpeedFactor(float val) { mSpeedFactor = val; }
	void setMaxSpeed(sf::Vector2f val) { mMaxSpeed = val; }
	void update(const sf::Time& dt) override;
private:
	PongPlatform* const mPlatform = nullptr;
	sf::Vector2f mDefaultPos;
	sf::Vector2f mTargetPos;
	sf::Vector2f mLastPos;
	float mVerticalMoveFactor = 0.95f;
	float mSpeedFactor = 50.f;
	sf::Vector2f mMaxSpeed;
};
