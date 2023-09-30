#pragma once
#include <queue>

#include "PlatformControllerBase.h"
#include "PongBall.h"
#include "UserPlatformContoller.h"

class AiPlatformController : public UserPlatformController {
public:
	explicit AiPlatformController(PongPlatform* platform);
	void beginObserve(weak_ptr<PongPlatform> opponentPlatform, weak_ptr<PongBall> ball);
	void update(const sf::Time& dt) override;
	void setReactionDelay(const sf::Time& delay) { mReactionDelay = delay; }
	void setObservePeriod(const sf::Time& period) { mObservePeriod = period; }

private:
	struct ObservedState {
		sf::Clock waitingTime;
		sf::Vector2f ballPos;
	};

	weak_ptr<PongPlatform> mOpponentPlatform;
	weak_ptr<PongBall> mBall;
	sf::Time mReactionDelay;
	sf::Time mObservePeriod;
	sf::Clock mObserveTimer;
	std::queue<ObservedState> mObservedStates;
};

inline void AiPlatformController::beginObserve(weak_ptr<PongPlatform> opponentPlatform, weak_ptr<PongBall> ball) {
	mOpponentPlatform = opponentPlatform;
	mBall = ball;
}