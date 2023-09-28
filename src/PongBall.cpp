#include "PongBall.h"

void PongBall::update(const sf::Time& dt) {
	ShapeBody::update(dt);

	auto pc = getPhysicalComponent();
	if (auto speedExcess = utils::length(pc->mVelocity) / mTargetSpeed; speedExcess > 1.f) {
		float dampingMultiplier = 1 - speedExcess * dt.asSeconds() * mSpeedDampingFactor;
		pc->mVelocity *= dampingMultiplier;
	}
}
