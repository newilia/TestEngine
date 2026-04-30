#include "Engine/Editor/NodeInspectorWidget.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <imgui.h>

#include <string>
#include <typeinfo>

namespace {
	void DrawIPropertiesProviderBlock(const char* title, Engine::IPropertiesProvider* inspectable,
	                                  const Engine::PropertyTreeDrawer& drawer) {
		if (!inspectable) {
			return;
		}
		Engine::PropertyTree tree;
		Engine::PropertyBuilder builder(tree);
		inspectable->BuildPropertyTree(builder);
		if (tree.roots.empty()) {
			return;
		}
		drawer.Draw(tree);
	}
} // namespace

namespace Engine {
	void NodeInspectorWidget::Draw(const std::shared_ptr<SceneNode>& node) const {
		if (!node) {
			ImGui::TextUnformatted("No node selected");
			return;
		}
		DrawIPropertiesProviderBlock("SceneNode", dynamic_cast<IPropertiesProvider*>(node.get()), _propertyDrawer);

		DrawIPropertiesProviderBlock("Transform", node->GetTransform().get(), _propertyDrawer);

		if (const auto visual = node->GetVisual()) {
			DrawIPropertiesProviderBlock("Visual", dynamic_cast<IPropertiesProvider*>(visual.get()), _propertyDrawer);
		}
		if (const auto sorting = node->GetSortingStrategy()) {
			DrawIPropertiesProviderBlock("Sorting strategy", dynamic_cast<IPropertiesProvider*>(sorting.get()),
			                             _propertyDrawer);
		}
		for (const auto& behaviour : node->GetBehaviours()) {
			if (!behaviour) {
				continue;
			}
			const std::string title = std::string("Behaviour: ") + typeid(*behaviour).name();
			DrawIPropertiesProviderBlock(title.c_str(), dynamic_cast<IPropertiesProvider*>(behaviour.get()),
			                             _propertyDrawer);
		}
	}
} // namespace Engine
