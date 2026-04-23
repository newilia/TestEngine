#include "BodyPullHandler.h"

#include "Engine/EngineInterface.h"
#include "Engine/Utils.h"
#include "Engine/VectorArrow.h"
#include "UserPullComponent.h"

void BodyPullHandler::startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode) {
	auto bodies = EngineContext::Instance().GetPhysicsHandler()->GetAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		if (!utils::isPointInsideOfBody(mousePos, body.get())) {
			continue;
		}
		if (body->GetPhysicalComponent()->isImmovable()) {
			if (pullMode == UserPullComponent::PullMode::FORCE || pullMode == UserPullComponent::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->RequireComponent<UserPullComponent>();
		pullComponent->_localSourcePoint = mousePos - body->GetPosGlobal();
		pullComponent->_globalDestPoint = mousePos;
		pullComponent->_mode = pullMode;
		_pullingBody = utils::sharedPtrCast<AbstractBody>(body.get());
		break;
	}
}

void BodyPullHandler::stopPull() {
	if (auto pullingBody = _pullingBody.lock()) {
		pullingBody->RemoveComponent<UserPullComponent>();
	}
	_pullingBody.reset();
}

void BodyPullHandler::setPullDestination(sf::Vector2f dest) const {
	if (auto pullingBody = _pullingBody.lock()) {
		if (auto pullComp = pullingBody->FindComponent<UserPullComponent>()) {
			pullComp->_globalDestPoint = dest;
		}
	}
}

void BodyPullHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isDebugDrawEnabled) {
		return;
	}

	if (auto body = _pullingBody.lock()) {
		if (auto pullComp = body->FindComponent<UserPullComponent>()) {
			if (pullComp->_mode == UserPullComponent::PullMode::FORCE) {
				VectorArrow arrow;
				arrow.setColor(sf::Color::Green);
				arrow.setStartPos(body->GetPosGlobal() + pullComp->_localSourcePoint);
				arrow.setEndPos(pullComp->_globalDestPoint);
				target.draw(arrow, states);
			}
		}
	}
}
