#include "BodyPullHandler.h"

#include "GlobalInterface.h"
#include "UserPullComponent.h"

void BodyPullHandler::onMouseButtonPress(const sf::Event::MouseButtonEvent& event) {
	if (event.button != sf::Mouse::Left && event.button != sf::Mouse::Right) {
		return;
	}
	sf::Vector2f pointerPos(event.x, event.y);
	shared_ptr<AbstractBody> bodyUnderCursor;
	auto bodies = GlobalInterface::getInstance()->getPhysicsHandler()->getAllBodies();
	for (auto wBody : bodies) {
		if (auto body = wBody.lock()) {
			if (utils::isPointInsideOfBody(pointerPos, body)) {
				bodyUnderCursor = body;
				mPullingBody = body;
				break;
			}
		}
	}	
	if (!bodyUnderCursor) {
		return;
	}
	auto pullComponent = bodyUnderCursor->requireComponent<UserPullComponent>();
	pullComponent->mSourcePoint = pointerPos - bodyUnderCursor->getPhysicalComponent()->mPos;
	pullComponent->mDestPoint = pointerPos;

	if (event.button == sf::Mouse::Left) {
		pullComponent->mMode = UserPullComponent::PullMode::SOFT;
	}
	else if (event.button == sf::Mouse::Right) {
		pullComponent->mMode = UserPullComponent::PullMode::HARD;
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
