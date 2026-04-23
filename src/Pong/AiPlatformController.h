#pragma once
#include "PlatformControllerBase.h"
#include "PongBall.h"
#include "UserPlatformContoller.h"

#include <SFML/System/Clock.hpp>

#include <memory>
#include <queue>

class AiPlatformController : public UserPlatformController
{
public:
	explicit AiPlatformController(PongPlatform* platform);
	void Update(const sf::Time& dt) override;

	void setReactionDelay(const sf::Time& delay) { _reactionDelay = delay; }

	void setObservePeriod(const sf::Time& period) { _observePeriod = period; }

	void beginObserve(const std::weak_ptr<PongPlatform>& opponentPlatform, const std::weak_ptr<PongBall>& ball);
	void setAggressivenessParams(float aggression, const sf::Time& changePeriod);

private:
	struct ExternalState
	{
		sf::Vector2f ballPos;
	};

	struct ExternalStateTimer
	{
		sf::Clock timePassed;
		ExternalState state;
	};

	struct InternalState
	{
		float activity = 0.1f;
	};

	void observeState();
	void react();

	// reactions
	void movePlatformTowardsBall();

	std::weak_ptr<PongPlatform> _opponentPlatform;
	std::weak_ptr<PongBall> _ball;
	sf::Time _reactionDelay;
	sf::Time _observePeriod;
	sf::Clock _observeTimer;
	std::queue<ExternalStateTimer> _externalStateTimers;
	ExternalState _curExState;
	ExternalState _prevExState;
	float _aggression = 0.5f;
	sf::Time _aggressionChangePeriod;
	sf::Clock _aggressionChangeTimer;
};