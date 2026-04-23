#include "BodyPullHandler.h"

#include "Engine/EngineInterface.h"
#include "Engine/Utils.h"
#include "Engine/VectorArrow.h"
#include "UserPullComponent.h"

void BodyPullHandler::startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode) {
	auto bodies = EngineContext::Instance().GetPhysicsHandler()->getAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		if (!utils::isPointInsideOfBody(mousePos, body.get())) {
			continue;
		}
		if (body->getPhysicalComponent()->isImmovable()) {
			if (pullMode == UserPullComponent::PullMode::FORCE || pullMode == UserPullComponent::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->requireComponent<UserPullComponent>();
		pullComponent->_localSourcePoint = mousePos - body->getPosGlobal();
		pullComponent->_globalDestPoint = mousePos;
		pullComponent->_mode = pullMode;
		_pullingBody = utils::sharedPtrCast<AbstractBody>(body.get());
		break;
	}
}

void BodyPullHandler::stopPull() {
	if (auto pullingBody = _pullingBody.lock()) {
		pullingBody->removeComponent<UserPullComponent>();
	}
	_pullingBody.reset();
}

void BodyPullHandler::setPullDestination(sf::Vector2f dest) const {
	if (auto pullingBody = _pullingBody.lock()) {
		if (auto pullComp = pullingBody->findComponent<UserPullComponent>()) {
			pullComp->_globalDestPoint = dest;
		}
	}
}

void BodyPullHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isDebugDrawEnabled) {
		return;
	}

	if (auto body = _pullingBody.lock()) {
		if (auto pullComp = body->findComponent<UserPullComponent>()) {
			if (pullComp->_mode == UserPullComponent::PullMode::FORCE) {
				VectorArrow arrow;
				arrow.setColor(sf::Color::Green);
				arrow.setStartPos(body->getPosGlobal() + pullComp->_localSourcePoint);
				arrow.setEndPos(pullComp->_globalDestPoint);
				target.draw(arrow, states);
			}
		}
	}
}
