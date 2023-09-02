#include "BodyPullHandler.h"

#include "GlobalInterface.h"
#include "UserPullComponent.h"

void BodyPullHandler::startPull(sf::Vector2f mousePos, UserPullComponent::PullMode pullMode) {
	auto bodies = GlobalInterface::getInstance()->getPhysicsHandler()->getAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		if (!utils::isPointInsideOfBody(mousePos, body)) {
			continue;
		}
		auto pullComponent = body->requireComponent<UserPullComponent>();
		pullComponent->mSourcePoint = mousePos - body->getPhysicalComponent()->mPos;
		pullComponent->mDestPoint = mousePos;
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
			pullComp->mDestPoint = dest;
		}
	}
}

void BodyPullHandler::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!mIsDebugDrawEnabled) {
		return;
	}
	
	if (auto body = mPullingBody.lock()) {
		if (auto pullComp = body->findComponent<UserPullComponent>()) {
			sf::Vertex vertices[2];
			vertices[0].position = body->getPhysicalComponent()->mPos + pullComp->mSourcePoint;
			vertices[1].position = pullComp->mDestPoint;
			vertices[0].color = sf::Color::White;
			vertices[1].color = sf::Color::White;
			target.draw(vertices, 2, sf::PrimitiveType::LineStrip, states);
		}
	}
}
