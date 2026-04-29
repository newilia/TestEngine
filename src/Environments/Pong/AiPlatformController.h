#pragma once
#include "PlatformControllerBase.h"
#include "PongBall.h"
#include "UserPlatformController.h"

#include <SFML/System/Clock.hpp>

#include <memory>
#include <queue>

class AiPlatformController : public UserPlatformController
{
public:
	explicit AiPlatformController(PongPlatform* platform);
	void Update(const sf::Time& dt) override;

	void SetReactionDelay(const sf::Time& delay);
	void SetObservePeriod(const sf::Time& period);

	void BeginObserve(const std::weak_ptr<PongPlatform>& opponentPlatform, const std::weak_ptr<PongBall>& ball);
	void SetAggressivenessParams(float aggression, const sf::Time& changePeriod);

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

	void ObserveState();
	void React();
	void MovePlatformTowardsBall();

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
