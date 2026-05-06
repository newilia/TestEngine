#include "UserPlatformControllerBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "PongPlatform.h"
#include "UserPlatformControllerBehaviour.generated.hpp"

void UserPlatformControllerBehaviour::SetMovementBounds(std::weak_ptr<SceneNode> movementRegionRect) {
	_movementBounds = std::move(movementRegionRect);
}

void UserPlatformControllerBehaviour::OnInit() {
	ResyncSpawnFromNode();
	EventHandlerBehaviourBase::OnInit();
}

void UserPlatformControllerBehaviour::ResyncSpawnFromNode() {
	if (auto p = GetNode()) {
		_targetPos = _defaultPos = Utils::GetWorldPos(p);
		ClampPongPlatformDesiredCenter(_targetPos, true, p, _movementBounds);
		ClampPongPlatformDesiredCenter(_defaultPos, true, p, _movementBounds);
		Utils::SetWorldPos(p, _defaultPos);
	}
}

void UserPlatformControllerBehaviour::OnDeinit() {
	EventHandlerBehaviourBase::OnDeinit();
}

void UserPlatformControllerBehaviour::OnUpdate(const sf::Time& /*dt*/) {
	auto p = GetNode();
	if (!p) {
		return;
	}
	ClampPongPlatformToPlayfield(p, true, _movementBounds);
	ClampPongPlatformDesiredCenter(_targetPos, true, p, _movementBounds);
	ApplyPongPlatformVelocityTowardsTarget(p, _targetPos, _speedFactor, _velLimit);
}

void UserPlatformControllerBehaviour::OnEvent(const sf::Event& event) {
	if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
		auto p = GetNode();
		if (!p) {
			return;
		}
		auto* window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return;
		}
		_targetPos = Utils::MapWindowPixelToWorld(*window, moved->position);
		ClampPongPlatformDesiredCenter(_targetPos, true, p, _movementBounds);
	}
}
