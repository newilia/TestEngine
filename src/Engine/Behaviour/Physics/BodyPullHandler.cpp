#include "BodyPullHandler.h"

#include "BodyPullHandler_gen.hpp"
#include "Engine/App/EngineContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/RigidBodyBehaviour.h"
#include "Engine/Core/SceneNode.h"

#include <algorithm>
#include <memory>

namespace {

	constexpr float kBasePullStrength = 100000.f;

	UserPullBehaviour::PullMode PullModeFromIndex(int index) {
		switch (std::clamp(index, 0, 2)) {
		case 0:
			return UserPullBehaviour::PullMode::POSITION;
		case 1:
			return UserPullBehaviour::PullMode::FORCE;
		default:
			return UserPullBehaviour::PullMode::VELOCITY;
		}
	}

} // namespace

BodyPullHandler::BodyPullHandler(std::shared_ptr<VectorArrowVisual> arrowVisual)
    : _arrowVisual(std::move(arrowVisual)) {}

void BodyPullHandler::EnableDebugDraw(bool enable) {
	_isDebugDrawEnabled = enable;
}

BodyPullSetup CreateBodyPullOverlay() {
	auto root = std::make_shared<SceneNode>();
	root->SetName("body_pull");

	auto arrowNode = std::make_shared<SceneNode>();
	arrowNode->SetName("body_pull_arrow");
	auto arrowVis = std::make_shared<VectorArrowVisual>();
	arrowNode->SetVisual(arrowVis);
	root->AddChild(std::move(arrowNode));

	auto handler = std::make_shared<BodyPullHandler>(arrowVis);
	root->AddBehaviour(handler);
	return {root, handler};
}

void BodyPullHandler::StartPull(sf::Vector2f mousePos) {
	StartPull(mousePos, PullModeFromIndex(_defaultPullModeIndex));
}

void BodyPullHandler::StartPull(sf::Vector2f mousePos, UserPullBehaviour::PullMode pullMode) {
	auto physicsProcessor = EngineContext::GetInstance().GetPhysicsProcessor();
	if (!physicsProcessor) {
		return;
	}
	auto bodies = physicsProcessor->GetAllBodies();
	for (auto wBody : bodies) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto* collider = body->FindShapeCollider();
		if (!collider || !Utils::IsPointInsideOfBody(mousePos, collider)) {
			continue;
		}
		auto rigidBody = body->FindBehaviour<RigidBodyBehaviour>();
		if (rigidBody && rigidBody->IsImmovable()) {
			if (pullMode == UserPullBehaviour::PullMode::FORCE || pullMode == UserPullBehaviour::PullMode::VELOCITY) {
				continue;
			}
		}
		auto pullComponent = body->RequireBehaviour<UserPullBehaviour>();
		pullComponent->_localSourcePoint = mousePos - body->GetPosGlobal();
		pullComponent->_globalDestPoint = mousePos;
		pullComponent->_mode = pullMode;
		pullComponent->_pullingStrength = kBasePullStrength * _pullForceScale;
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
		if (auto pullComp = pullingBody->FindBehaviour<UserPullBehaviour>()) {
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
		if (auto pullComp = body->FindBehaviour<UserPullBehaviour>()) {
			if (pullComp->_mode == UserPullBehaviour::PullMode::FORCE) {
				_arrowVisual->SetColor(sf::Color::Green);
				_arrowVisual->SetStartPos(body->GetPosGlobal() + pullComp->_localSourcePoint);
				_arrowVisual->SetEndPos(pullComp->_globalDestPoint);
				_arrowVisual->SetVisible(true);
				return;
			}
		}
	}
	_arrowVisual->SetVisible(false);
}
