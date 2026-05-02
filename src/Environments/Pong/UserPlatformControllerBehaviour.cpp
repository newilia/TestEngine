#include "UserPlatformControllerBehaviour.h"

#include "PongPlatform.h"
#include "UserPlatformControllerBehaviour.generated.hpp"

void UserPlatformControllerBehaviour::OnInit() {
	if (auto p = GetNode()) {
		_targetPos = _defaultPos = p->GetPosGlobal();
		ClampPongPlatformDesiredCenter(_targetPos, true, p);
		ClampPongPlatformDesiredCenter(_defaultPos, true, p);
		p->SetPosGlobal(_defaultPos);
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
	ClampPongPlatformToPlayfield(p, true);
	ClampPongPlatformDesiredCenter(_targetPos, true, p);
	ApplyPongPlatformVelocityTowardsTarget(p, _targetPos, _speedFactor, _velLimit);
}

void UserPlatformControllerBehaviour::OnUserInput(const sf::Event& event) {
	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		auto p = GetNode();
		if (!p) {
			return;
		}
		_targetPos = sf::Vector2f(static_cast<float>(moved->position.x), static_cast<float>(moved->position.y));
		ClampPongPlatformDesiredCenter(_targetPos, true, p);
	}
}
