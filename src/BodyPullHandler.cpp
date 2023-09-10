#include "BodyPullHandler.h"

#include "EngineInterface.h"
#include "UserPullComponent.h"
#include "VectorArrow.h"

void BodyPullHandler::startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode) {
	auto bodies = EI()->getPhysicsHandler()->getAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		if (!utils::isPointInsideOfBody(mousePos, body)) {
			continue;
		}
		if (body->getPhysicalComponent()->isImmovable()) {
			if (pullMode == UserPullComponent::PullMode::FORCE || pullMode == UserPullComponent::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->requireComponent<UserPullComponent>();
		pullComponent->mLocalSourcePoint = mousePos - body->getPosGlobal();
		pullComponent->mGlobalDestPoint = mousePos;
		pullComponent->mMode = pullMode;
		mPullingBody = body;
		break;
	}
}

void BodyPullHandler::stopPull() {
	if (auto pullingBody = mPullingBody.lock()) {
		pullingBody->removeComponent<UserPullComponent>();
	}
	mPullingBody.reset();
}

void BodyPullHandler::setPullDestination(sf::Vector2f dest) const {
	if (auto pullingBody = mPullingBody.lock()) {
		if (auto pullComp = pullingBody->findComponent<UserPullComponent>()) {
			pullComp->mGlobalDestPoint = dest;
		}
	}
}

void BodyPullHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!mIsDebugDrawEnabled) {
		return;
	}
	
	if (auto body = mPullingBody.lock()) {
		if (auto pullComp = body->findComponent<UserPullComponent>()) {
			if (pullComp->mMode == UserPullComponent::PullMode::FORCE) {
				VectorArrow arrow;
				arrow.setColor(sf::Color::Green);
				arrow.setStartPos(body->getPosGlobal() + pullComp->mLocalSourcePoint);
				arrow.setEndPos(pullComp->mGlobalDestPoint);
				target.draw(arrow, states);
			}
		}
	}
}
