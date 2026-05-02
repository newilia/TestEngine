#include "UserPlatformControllerBehaviour.h"

#include "PongPlatform.h"
#include "UserPlatformControllerBehaviour.generated.hpp"

#include <cmath>

void UserPlatformControllerBehaviour::OnInit() {
	if (auto p = GetNode()) {
		_targetPos = _defaultPos = p->GetPosGlobal();
	}
	InputHandlerBehaviourBase::OnInit();
}

void UserPlatformControllerBehaviour::OnDeinit() {
	InputHandlerBehaviourBase::OnDeinit();
}

void UserPlatformControllerBehaviour::OnUpdate(const sf::Time& /*dt*/) {
	auto p = GetNode();
	if (!p) {
		return;
	}
	ApplyPongPlatformVelocityTowardsTarget(p, _targetPos, _speedFactor, _velLimit);
}

void UserPlatformControllerBehaviour::OnUserInput(const sf::Event& event) {
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
