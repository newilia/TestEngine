#include "ScreenRectSetterBehaviour.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Visual/Visual.h"
#include "ScreenRectSetterBehaviour.generated.hpp"

void ScreenRectSetterBehaviour::OnInit() {
	const auto node = GetNode();
	if (!node) {
		return;
	}
	const auto visual = node->GetVisual();
	if (!visual) {
		return;
	}
	const sf::FloatRect worldRect = node->GetWorldTransform().transformRect(visual->GetGlobalBounds());
	Engine::MainContext::GetInstance().FocusCameraOnWorldRect(worldRect, false);
}
