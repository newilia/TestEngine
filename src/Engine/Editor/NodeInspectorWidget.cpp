#include "Engine/Editor/NodeInspectorWidget.h"

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/EditorVisualTheme.h"
#include "Engine/Editor/SceneCloneUtils.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/Visual.h"

#include <fmt/format.h>
#include <imgui.h>

#include <string>
#include <typeinfo>

namespace {
	void DrawIPropertiesProviderBlock(Engine::EditorVisualTheme::InspectorSectionHeaderStyle sectionStyle,
	                                  const char* title, Engine::IPropertiesProvider* inspectable,
	                                  const std::shared_ptr<EntityOnNode>& entity, Engine::EntitySlot slot,
	                                  const Engine::PropertyTreeDrawer& drawer) {
		if (!inspectable) {
			return;
		}
		Engine::PropertyTree tree;
		Engine::PropertyBuilder builder(tree);
		inspectable->BuildPropertyTree(builder);
		if (tree.roots.empty() && tree.inspectorMethods.empty() && !entity) {
			return;
		}
		ImGui::PushID(static_cast<const void*>(inspectable));
		Engine::EditorVisualTheme::PushInspectorSectionHeaderColors(sectionStyle);
		const bool open = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen);
		Engine::EditorVisualTheme::PopInspectorSectionHeaderColors();
		if (!tree.inspectorMethods.empty() || entity) {
			if (ImGui::BeginPopupContextItem("inspector_reflected_methods", ImGuiPopupFlags_MouseButtonRight)) {
				if (entity) {
					if (ImGui::MenuItem("Copy")) {
						(void)Engine::Editor::GetInstance().CopyEntity(entity, slot);
					}
					const bool canCutOrDelete = (slot != Engine::EntitySlot::Transform);
					if (!canCutOrDelete) {
						ImGui::BeginDisabled();
					}
					if (ImGui::MenuItem("Cut")) {
						(void)Engine::Editor::GetInstance().CutEntity(entity, slot);
					}
					if (ImGui::MenuItem("Delete")) {
						(void)Engine::Editor::GetInstance().DeleteEntity(entity, slot);
					}
					if (!canCutOrDelete) {
						ImGui::EndDisabled();
					}
					const bool canPaste = Engine::Editor::GetInstance().CanPasteEntityToSelectedNode();
					if (!canPaste) {
						ImGui::BeginDisabled();
					}
					if (ImGui::MenuItem("Paste")) {
						(void)Engine::Editor::GetInstance().PasteClipboard();
					}
					if (!canPaste) {
						ImGui::EndDisabled();
					}
					if (!tree.inspectorMethods.empty()) {
						ImGui::Separator();
					}
				}

				for (const auto& m : tree.inspectorMethods) {
					if (ImGui::MenuItem(fmt::format("Call {}", m.menuLabel).c_str()) && m.invoke) {
						m.invoke();
					}
				}

				ImGui::EndPopup();
			}
		}
		if (!open) {
			ImGui::PopID();
			return;
		}
		if (!tree.roots.empty()) {
			drawer.Draw(tree, {.unwrapSingleRootObject = true});
		}
		ImGui::PopID();
	}
} // namespace

namespace Engine {
	void NodeInspectorWidget::Draw(const std::shared_ptr<SceneNode>& node) const {
		if (!node) {
			ImGui::TextUnformatted("No node selected");
			return;
		}
		DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SceneNode, "SceneNode",
		                             dynamic_cast<IPropertiesProvider*>(node.get()), nullptr, EntitySlot::Behaviour,
		                             _propertyDrawer);

		DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Transform, "Transform",
		                             node->GetLocalTransform().get(), node->GetLocalTransform(), EntitySlot::Transform,
		                             _propertyDrawer);

		if (const auto sorting = node->GetSortingStrategy()) {
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::SortingStrategy,
			                             "Sorting strategy", dynamic_cast<IPropertiesProvider*>(sorting.get()), sorting,
			                             EntitySlot::SortingStrategy, _propertyDrawer);
		}
		if (const auto visual = node->GetVisual()) {
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Visual, "Visual",
			                             dynamic_cast<IPropertiesProvider*>(visual.get()), visual, EntitySlot::Visual,
			                             _propertyDrawer);
		}
		for (const auto& behaviour : node->GetBehaviours()) {
			if (!behaviour) {
				continue;
			}
			const std::string behTitle = fmt::format("Behaviour ({})", typeid(*behaviour).name());
			DrawIPropertiesProviderBlock(EditorVisualTheme::InspectorSectionHeaderStyle::Behaviour, behTitle.c_str(),
			                             dynamic_cast<IPropertiesProvider*>(behaviour.get()), behaviour,
			                             EntitySlot::Behaviour, _propertyDrawer);
		}
	}
} // namespace Engine
