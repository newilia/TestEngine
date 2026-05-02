#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "PongBall.h"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <memory>
#include <queue>

/// AI paddle: observes ball with delay and moves toward predicted position.
class AiPlatformControllerBehaviour : public Behaviour
{
	META_CLASS()

public:
	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;

	void SetReactionDelay(sf::Time delay);
	void SetObservePeriod(sf::Time period);
	void SetAggressivenessParams(float aggression, sf::Time changePeriod);
	void BeginObserve(const std::weak_ptr<SceneNode>& opponentPlatformNode, const std::weak_ptr<PongBall>& ball);

	void ResyncSpawnFromNode();
	void ClearPendingObservations();

	/* TODO incapsulation */
	/// @property(minValue=0.f, maxValue=100000.f, tooltip="Scales velocity toward target position.")
	float _speedFactor = 50.f;
	/// @property(tooltip="Max velocity components (pixels/sec style scale).")
	sf::Vector2f _velLimit = {3000.f, 1000.f};
	/// @property(minValue=0.f, tooltip="Seconds before reacting to an observed ball state.")
	float _reactionDelaySeconds = 0.1f;
	/// @property(minValue=0.f, tooltip="Seconds between ball observations.")
	float _observePeriodSeconds = 0.01f;
	/// @property(minValue=0.f, maxValue=1.f)
	float _aggression = 0.5f;
	/// @property(minValue=0.f, tooltip="Seconds between aggression rerolls.")
	float _aggressionChangePeriodSeconds = 1.f;

private:
	void ObserveState();
	void React();
	void MovePlatformTowardsBall();

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

	sf::Vector2f _defaultPos{}; // TODO what is this
	sf::Vector2f _targetPos{};

	std::weak_ptr<SceneNode> _opponentPlatform;
	std::weak_ptr<PongBall> _ball;
	sf::Clock _observeTimer;
	std::queue<ExternalStateTimer> _externalStateTimers;
	ExternalState _curExState{};
	ExternalState _prevExState{};
	sf::Clock _aggressionChangeTimer;
};
