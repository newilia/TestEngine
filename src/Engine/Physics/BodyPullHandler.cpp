#include "BodyPullHandler.h"

#include "Engine/EngineInterface.h"
#include "Engine/SceneNode.h"
#include "Engine/Utils.h"
#include "Engine/VectorArrow.h"

void BodyPullHandler::StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode) {
	auto bodies = EngineContext::Instance().GetPhysicsHandler()->GetAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto* collider = body->FindShapeCollider();
		if (!collider || !utils::isPointInsideOfBody(mousePos, collider)) {
			continue;
		}
		if (body->GetPhysicalComponent()->isImmovable()) {
			if (pullMode == UserPullBehaviour::PullMode::FORCE || pullMode == UserPullBehaviour::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->RequireEntity<UserPullBehaviour>();
		pullComponent->_localSourcePoint = mousePos - body->GetPosGlobal();
		pullComponent->_globalDestPoint = mousePos;
		pullComponent->_mode = pullMode;
		_pullingBody = body;
		break;
	}
}

void BodyPullHandler::StopPull() {
	if (auto pullingBody = _pullingBody.lock()) {
		pullingBody->RemoveBehaviour<UserPullBehaviour>();
	}
	_pullingBody.reset();
}

void BodyPullHandler::SetPullDestination(sf::Vector2f dest) const {
	if (auto pullingBody = _pullingBody.lock()) {
		if (auto pullComp = pullingBody->FindEntity<UserPullBehaviour>()) {
			pullComp->_globalDestPoint = dest;
		}
	}
}

void BodyPullHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_isDebugDrawEnabled) {
		return;
	}

	if (auto body = _pullingBody.lock()) {
		if (auto pullComp = body->FindEntity<UserPullBehaviour>()) {
			if (pullComp->_mode == UserPullBehaviour::PullMode::FORCE) {
				VectorArrow arrow;
				arrow.setColor(sf::Color::Green);
				arrow.setStartPos(body->GetPosGlobal() + pullComp->_localSourcePoint);
				arrow.setEndPos(pullComp->_globalDestPoint);
				target.draw(arrow, states);
			}
		}
	}
}
