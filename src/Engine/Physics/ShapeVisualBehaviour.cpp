#include "ShapeVisualBehaviour.h"

#include "ShapeColliderBehaviourBase.h"
#include "ShapeVisual.h"
#include "Engine/Core/SceneNode.h"

void ShapeVisualBehaviour::OnAttached() {
	auto node = GetNode();
	if (!node) {
		return;
	}
	auto* collider = node->FindShapeCollider();
	if (!collider) {
		return;
	}
	auto visual = MakeShapeVisual(collider->GetBaseShape());
	if (visual) {
		node->SetVisual(std::move(visual));
	}
}
