#include "ShapeVisualBehaviour.h"

#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/MakeShapeVisual.h"
#include "ShapeColliderBehaviourBase.h"

void ShapeVisualBehaviour::OnInit() {
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
		_ownsShapeVisual = true;
	}
}

void ShapeVisualBehaviour::OnDeinit() {
	if (!_ownsShapeVisual) {
		return;
	}
	if (auto node = GetNode()) {
		node->SetVisual(nullptr);
	}
	_ownsShapeVisual = false;
}
