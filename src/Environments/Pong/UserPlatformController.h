#pragma once
#include "Engine/Core/Updatable.h"
#include "PlatformControllerBase.h"

#include <SFML/System/Vector2.hpp>

class PongPlatform;

namespace sf {
	class Event;
}

class UserPlatformController : public PlatformControllerBase
{
public:
	explicit UserPlatformController(PongPlatform* platform) : PlatformControllerBase(platform) {}

	void Init() override;
	void HandleEvent(const sf::Event& event);
	void Update(const sf::Time& dt) override;

	void SetVerticalFreedomFactor(float val) { _verticalMoveFactor = val; }

	void SetVelocityFactor(float val) { _speedFactor = val; }

	void SetVelLimit(sf::Vector2f val) { _velLimit = val; }

protected:
	sf::Vector2f _defaultPos;
	sf::Vector2f _targetPos;
	sf::Vector2f _lastPos;
	float _verticalMoveFactor = 0.9f;
	float _speedFactor = 50.f;
	sf::Vector2f _velLimit;
};
