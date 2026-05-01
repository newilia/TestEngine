#include "ScenePickUtils.h"

#include "Engine/App/MainContext.h"
#include "Engine/App/Utils.h"
#include "Engine/Behaviour/Physics/ShapeColliderBehaviourBase.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <memory>

std::shared_ptr<SceneNode> PickSceneNodeAt(const std::shared_ptr<Scene>& scene, const sf::Vector2f& worldPoint) {
	if (!scene) {
		return nullptr;
	}
	if (auto hit = scene->FindTopMostNodeAtPoint(worldPoint)) {
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
		if (!collider || !Utils::IsWorldPointInsideOfBody(worldPoint, collider)) {
			continue;
		}
		return body;
	}
	return nullptr;
}
