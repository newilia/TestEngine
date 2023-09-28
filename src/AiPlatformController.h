#pragma once
#include "PlatformControllerBase.h"
#include "PongBall.h"

class AiPlatformController : public PlatformControllerBase {
public:
	explicit AiPlatformController(PongPlatform* platform);
	void beginObserve(weak_ptr<PongPlatform> opponentPlatform, weak_ptr<PongBall> ball);
	void update(const sf::Time& dt) override;
private:
	weak_ptr<PongPlatform> mOpponentPlatform;
	weak_ptr<PongBall> mBall;
};

inline void AiPlatformController::beginObserve(weak_ptr<PongPlatform> opponentPlatform, weak_ptr<PongBall> ball) {
	mOpponentPlatform = opponentPlatform;
	mBall = ball;
}