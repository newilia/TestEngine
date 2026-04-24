#include "BodyPullHandler.h"

#include "Engine/App/EngineInterface.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/App/Utils.h"

#include <memory>

BodyPullHandler::BodyPullHandler(std::shared_ptr<VectorArrowNodeVisual> arrowVisual)
    : _arrowVisual(std::move(arrowVisual)) {}

BodyPullSetup CreateBodyPullOverlay() {
	auto root = std::make_shared<SceneNode>();
	root->setName("body_pull");

	auto arrowNode = std::make_shared<SceneNode>();
	arrowNode->setName("body_pull_arrow");
	auto arrowVis = std::make_shared<VectorArrowNodeVisual>();
	arrowNode->SetVisual(arrowVis);
	root->addChild(std::move(arrowNode));

	auto handler = std::make_shared<BodyPullHandler>(arrowVis);
	root->AddBehaviour(handler);
	return {root, handler};
}

void BodyPullHandler::StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode) {
	auto physicsHandler = EngineContext::Instance().GetPhysicsHandler();
	if (!physicsHandler) {
		return;
	}
	auto bodies = physicsHandler->GetAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto* collider = body->FindShapeCollider();
		if (!collider || !utils::isPointInsideOfBody(mousePos, collider)) {
			continue;
		}
		auto physComp = body->GetPhysicalComponent();
		if (physComp && physComp->isImmovable()) {
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

void BodyPullHandler::OnUpdate(const sf::Time& /*dt*/) {
	if (!_arrowVisual) {
		return;
	}
	if (!_isDebugDrawEnabled) {
		return;
	}
	if (auto body = _pullingBody.lock()) {
		if (auto pullComp = body->FindEntity<UserPullBehaviour>()) {
			if (pullComp->_mode == UserPullBehaviour::PullMode::FORCE) {
				_arrowVisual->setColor(sf::Color::Green);
				_arrowVisual->setStartPos(body->GetPosGlobal() + pullComp->_localSourcePoint);
				_arrowVisual->setEndPos(pullComp->_globalDestPoint);
				_arrowVisual->setVisible(true);
				return;
			}
		}
	}
	_arrowVisual->setVisible(false);
}
