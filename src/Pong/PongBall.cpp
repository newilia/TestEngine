#include "PongBall.h"

#include "Engine/Utils.h"

void PongBall::update(const sf::Time& dt) {
	ShapeBody::update(dt);

	auto pc = getPhysicalComponent();
	if (auto speedExcess = utils::length(pc->_velocity) / _targetSpeed; speedExcess > 1.f) {
		float dampingMultiplier = 1 - speedExcess * dt.asSeconds() * _speedDampingFactor;
		pc->_velocity *= dampingMultiplier;
	}
}
