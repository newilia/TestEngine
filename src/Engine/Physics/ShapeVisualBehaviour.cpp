#include "ShapeVisualBehaviour.h"

#include "AbstractShapeBody.h"
#include "ShapeNodeVisual.h"
#include "Engine/SceneNode.h"

void ShapeVisualBehaviour::OnAttached() {
	auto node = GetNode();
	if (!node || !_body) {
		return;
	}
	auto visual = std::make_shared<ShapeNodeVisual>(_body->GetBaseShape());
	node->SetVisual(std::move(visual));
}
