#include "ScenePickUtils.h"

#include "Engine/App/EngineContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <memory>

std::shared_ptr<SceneNode> PickSceneNodeAt(const std::shared_ptr<Scene>& scene, sf::Vector2f windowPosition) {
	if (!scene) {
		return nullptr;
	}
	if (auto hit = scene->FindTopMostTapTarget(windowPosition)) {
		return hit;
	}
	auto physicsProcessor = Engine::MainContext::GetInstance().GetPhysicsProcessor();
	if (!physicsProcessor) {
		return nullptr;
	}
	for (auto wBody : physicsProcessor->GetAllBodies()) {
		auto body = wBody.lock();
		if (!body) {
			continue;
		}
		auto* collider = body->FindShapeCollider();
		if (!collider || !Utils::IsPointInsideOfBody(windowPosition, collider)) {
			continue;
		}
		return body;
	}
	return nullptr;
}
