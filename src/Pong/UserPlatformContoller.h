#pragma once
#include "Engine/Updateable.h"
#include "PlatformControllerBase.h"

#include <SFML/System/Vector2.hpp>

class PongPlatform;

namespace sf {
class Event;
}

class UserPlatformController : public PlatformControllerBase
{
public:
	UserPlatformController(PongPlatform* platform) : PlatformControllerBase(platform) {};
	void Init() override;
	void handleEvent(const sf::Event& event);

	void setVerticalFreedomFactor(float val) { _verticalMoveFactor = val; }

	void setVelocityFactor(float val) { _speedFactor = val; }

	void setVelLimit(sf::Vector2f val) { _velLimit = val; }

	void Update(const sf::Time& dt) override;

protected:
	sf::Vector2f _defaultPos;
	sf::Vector2f _targetPos;
	sf::Vector2f _lastPos;
	float _verticalMoveFactor = 0.9f;
	float _speedFactor = 50.f;
	sf::Vector2f _velLimit;
};
