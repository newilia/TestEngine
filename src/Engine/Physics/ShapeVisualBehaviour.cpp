#include "ShapeVisualBehaviour.h"

#include "ShapeColliderBehaviourBase.h"
#include "ShapeNodeVisual.h"
#include "Engine/SceneNode.h"

void ShapeVisualBehaviour::OnAttached() {
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto* collider = node->FindShapeCollider();
	if (!collider) {
		return;
	}
	auto visual = std::make_shared<ShapeNodeVisual>(collider->GetBaseShape());
	node->SetVisual(std::move(visual));
}
