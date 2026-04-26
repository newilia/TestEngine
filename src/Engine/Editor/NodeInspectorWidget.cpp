#include "Engine/Editor/NodeInspectorWidget.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
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
		ImGui::SeparatorText(title);
		drawer.Draw(tree);
	}

} // namespace

namespace Engine {

	void NodeInspectorWidget::Draw(const std::shared_ptr<SceneNode>& node) const {
		if (!node) {
			ImGui::TextUnformatted("No node selected");
			return;
		}
		ImGui::SeparatorText("Node");
		DrawIPropertiesProviderBlock("SceneNode", dynamic_cast<IPropertiesProvider*>(node.get()), _propertyDrawer);
		ImGui::Text("Children: %zu", node->GetChildren().size());
		if (const auto parent = node->GetParent()) {
			const char* pName = parent->GetName().empty() ? "<unnamed>" : parent->GetName().c_str();
			ImGui::Text("Parent: ");
			ImGui::SameLine();
			ImGui::TextUnformatted(pName);
		}
		else {
			ImGui::TextUnformatted("Parent: (root)");
		}

		ImGui::SeparatorText("Transform");
		const sf::Vector2f local = node->getPosition();
		const sf::Vector2f world = node->GetPosGlobal();
		const sf::Vector2f scale = node->getScale();
		ImGui::Text("Local position:  (%.2f, %.2f)", static_cast<double>(local.x), static_cast<double>(local.y));
		ImGui::Text("Global position: (%.2f, %.2f)", static_cast<double>(world.x), static_cast<double>(world.y));
		ImGui::Text("Scale: (%.3f, %.3f)", static_cast<double>(scale.x), static_cast<double>(scale.y));
		const double rotDeg = static_cast<double>(node->getRotation().asDegrees());
		ImGui::Text("Rotation: %.2f deg", rotDeg);

		ImGui::SeparatorText("Entities");
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
