#include "AiPlatformController.h"

#include "PongPlatform.h"

AiPlatformController::AiPlatformController(PongPlatform* platform) : PlatformControllerBase(platform) {

}

void AiPlatformController::update(const sf::Time& dt) {
	if (auto ball = mBall.lock()) {
		auto pos = mPlatform->getShape()->getPosition();
		pos.x = ball->getShape()->getPosition().x;
		mPlatform->getShape()->setPosition(pos);
	}
}
