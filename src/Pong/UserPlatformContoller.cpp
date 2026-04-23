#include "UserPlatformContoller.h"

#include "Engine/EngineInterface.h"
#include "PongPlatform.h"
#include "SFML/Window/Event.hpp"

void UserPlatformController::init() {
	_targetPos = _defaultPos = _platform->getPosGlobal();
}

void UserPlatformController::handleEvent(const sf::Event& event) {
	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		float mouseYShift = static_cast<float>(moved->position.y) - _defaultPos.y;
		float platformYShift = 0.f;
		for (int i = 0; i < std::abs(mouseYShift); ++i) {
			platformYShift += powf(_verticalMoveFactor, static_cast<float>(i));
		}
		_targetPos.x = static_cast<float>(moved->position.x);
		_targetPos.y = _defaultPos.y + copysignf(platformYShift, mouseYShift);
	}
}

void UserPlatformController::update(const sf::Time& dt) {
	auto vel = (_targetPos - _platform->getShape()->getPosition()) * _speedFactor;
	vel.x = std::clamp(vel.x, -_velLimit.x, _velLimit.x);
	vel.y = std::clamp(vel.y, -_velLimit.y, _velLimit.y);
	_platform->getPhysicalComponent()->_velocity = vel;
}
