#include "BodyPullHandler.h"
#include "UserPullComponent.h"

void BodyPullHandler::onMouseButtonPress(const sf::Event::MouseButtonEvent& event) {
	sf::Vector2f pointerPos(event.x, event.y);

	auto bodies = PhysicsHandler::getInstance()->getAllBodies();
	for (auto wBody : bodies) {
		if (auto body = wBody.lock()) {
			if (utils::isPointInsideOfBody(pointerPos, body)) {
				mPullingBody = body;
				auto pullComponent = body->requireComponent<UserPullComponent>();
				pullComponent->mSourcePoint = pointerPos - body->getPhysicalComponent()->mPos;
				pullComponent->mDestPoint = pointerPos;
				break;
			}
		}
	}
}

void BodyPullHandler::onMouseButtonRelease(const sf::Event::MouseButtonEvent& event) {
	if (auto pullingBody = mPullingBody.lock()) {
		pullingBody->removeComponent<UserPullComponent>();
	}
	mPullingBody.reset();
}

void BodyPullHandler::onMouseMove(const sf::Event::MouseMoveEvent& event) {
	if (auto pullingBody = mPullingBody.lock()) {
		if (auto pullComp = pullingBody->findComponent<UserPullComponent>()) {
			pullComp->mDestPoint = sf::Vector2f(event.x, event.y);
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
