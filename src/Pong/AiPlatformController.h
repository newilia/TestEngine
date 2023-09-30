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
	void beginObserve(std::weak_ptr<PongPlatform> opponentPlatform, std::weak_ptr<PongBall> ball);
	void update(const sf::Time& dt) override;
	void setReactionDelay(const sf::Time& delay) { mReactionDelay = delay; }
	void setObservePeriod(const sf::Time& period) { mObservePeriod = period; }

private:
	void observeState();

	struct ObservedState {
		sf::Clock waitingTime;
		sf::Vector2f ballPos;
	};

	std::weak_ptr<PongPlatform> mOpponentPlatform;
	std::weak_ptr<PongBall> mBall;
	sf::Time mReactionDelay;
	sf::Time mObservePeriod;
	sf::Clock mObserveTimer;
	std::queue<ObservedState> mObservedStates;
};

inline void AiPlatformController::beginObserve(std::weak_ptr<PongPlatform> opponentPlatform, std::weak_ptr<PongBall> ball) {
	mOpponentPlatform = opponentPlatform;
	mBall = ball;
}