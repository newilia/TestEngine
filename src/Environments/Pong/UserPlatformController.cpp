#include "UserPlatformController.h"

#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "PongPlatform.h"
#include "SFML/Window/Event.hpp"

UserPlatformController::UserPlatformController(PongPlatform* platform) : PlatformControllerBase(platform) {}

void UserPlatformController::SetVerticalFreedomFactor(float val) {
	_verticalMoveFactor = val;
}

void UserPlatformController::SetVelocityFactor(float val) {
	_speedFactor = val;
}

void UserPlatformController::SetVelLimit(sf::Vector2f val) {
	_velLimit = val;
}

void UserPlatformController::Init() {
	_targetPos = _defaultPos = _platform->GetPosGlobal();
}

void UserPlatformController::HandleEvent(const sf::Event& event) {
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

void UserPlatformController::Update(const sf::Time& dt) {
	auto vel = (_targetPos - _platform->GetShape()->getPosition()) * _speedFactor;
	vel.x = std::clamp(vel.x, -_velLimit.x, _velLimit.x);
	vel.y = std::clamp(vel.y, -_velLimit.y, _velLimit.y);
	_platform->GetNode()->RequireBehaviour<RigidBodyBehaviour>()->_velocity = vel;
}
