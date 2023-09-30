#pragma once
#include <memory>
#include <queue>
#include <SFML/System/Clock.hpp>

#include "PlatformControllerBase.h"
#include "PongBall.h"
#include "UserPlatformContoller.h"

class AiPlatformController : public UserPlatformController {
public:
	explicit AiPlatformController(PongPlatform* platform);
	void update(const sf::Time& dt) override;
	void setReactionDelay(const sf::Time& delay) { mReactionDelay = delay; }
	void setObservePeriod(const sf::Time& period) { mObservePeriod = period; }
	void beginObserve(const std::weak_ptr<PongPlatform>& opponentPlatform, const std::weak_ptr<PongBall>& ball);
	void setAggressivenessParams(float aggression, const sf::Time& changePeriod);

private:
	struct ExternalState {
		sf::Vector2f ballPos;
	};

	struct ExternalStateTimer {
		sf::Clock timePassed;
		ExternalState state;
	};

	struct InternalState {
		float activity = 0.1f;
	};

	void observeState();
	void react();

	// reactions
	void movePlatformTowardsBall();

	std::weak_ptr<PongPlatform> mOpponentPlatform;
	std::weak_ptr<PongBall> mBall;
	sf::Time mReactionDelay;
	sf::Time mObservePeriod;
	sf::Clock mObserveTimer;
	std::queue<ExternalStateTimer> mExternalStateTimers;
	ExternalState mCurExState;
	ExternalState mPrevExState;
	float mAggression = 0.5f;
	sf::Time mAggressionChangePeriod;
	sf::Clock mAggressionChangeTimer;
};