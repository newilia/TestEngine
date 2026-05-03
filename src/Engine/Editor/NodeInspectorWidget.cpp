#include "Engine/Editor/NodeInspectorWidget.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/EditorVisualTheme.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <fmt/format.h>
#include <imgui.h>

#include <string>
#include <typeinfo>

namespace {
	void DrawIPropertiesProviderBlock(Engine::EditorVisualTheme::InspectorSectionHeaderStyle sectionStyle,
	                                  const char* title, Engine::IPropertiesProvider* inspectable,
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
		Engine::EditorVisualTheme::PushInspectorSectionHeaderColors(sectionStyle);
		const bool open = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
		Engine::EditorVisualTheme::PopInspectorSectionHeaderColors();
		if (!open) {
			return;
		}
		drawer.Draw(tree, {.unwrapSingleRootObject = true});
	}
} // namespace

namespace Engine {
	void NodeInspectorWidget::Draw(const std::shared_ptr<SceneNode>& node) const {
		if (!node) {
			ImGui::TextUnformatted("No node selected");
			return;
		}
		DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SceneNode, "SceneNode",
		                             dynamic_cast<IPropertiesProvider*>(node.get()), _propertyDrawer);

		DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Transform, "Transform",
		                             node->GetLocalTransform().get(), _propertyDrawer);

		if (const auto sorting = node->GetSortingStrategy()) {
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SortingStrategy,
			                             "Sorting strategy", dynamic_cast<IPropertiesProvider*>(sorting.get()),
			                             _propertyDrawer);
		}
		if (const auto visual = node->GetVisual()) {
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Visual, "Visual",
			                             dynamic_cast<IPropertiesProvider*>(visual.get()), _propertyDrawer);
		}
		for (const auto& behaviour : node->GetBehaviours()) {
			if (!behaviour) {
				continue;
			}
			const std::string behTitle = fmt::format("Behaviour ({})", typeid(*behaviour).name());
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Behaviour, behTitle.c_str(),
			                             dynamic_cast<IPropertiesProvider*>(behaviour.get()), _propertyDrawer);
		}
	}
} // namespace Engine
