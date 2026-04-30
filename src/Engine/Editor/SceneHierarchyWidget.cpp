#include "Engine/Editor/SceneHierarchyWidget.h"

#include <imgui.h>

namespace Engine {
	std::shared_ptr<SceneNode> SceneHierarchyWidget::GetSelected() const {
		return _selectedNode.lock();
	}

	void SceneHierarchyWidget::ClearSelection() {
		_selectedNode.reset();
	}

	void SceneHierarchyWidget::Select(std::shared_ptr<SceneNode> node) {
		_selectedNode = std::move(node);
	}

	void SceneHierarchyWidget::DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth) {
		const std::string& name = node.GetName();
		const char* displayCStr = name.empty() ? emptyNamePlaceholder : name.c_str();
		const bool hasChildren = !node.GetChildren().empty();
		const std::shared_ptr<SceneNode> currentSelection = _selectedNode.lock();
		const bool isSelected = (currentSelection.get() == &node);
		void* const id = static_cast<void*>(std::addressof(node));

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth;
		if (isSelected) {
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}
		if (hasChildren) {
			if (depth == 0) {
				nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
			}
		}
		else {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
		}

		const bool isOpen = ImGui::TreeNodeEx(id, nodeFlags, "%s", displayCStr);
		if (ImGui::IsItemClicked()) {
			_selectedNode = node.shared_from_this();
		}
		if (isOpen) {
			if (hasChildren) {
				++depth;
				for (const auto& child : node.GetChildren()) {
					if (child) {
						DrawNode(*child, "<unnamed>", depth);
					}
				}
			}
			ImGui::TreePop();
		}
	}

	void SceneHierarchyWidget::Draw(const std::shared_ptr<Scene>& scene) {
		if (!scene) {
			ClearSelection();
			ImGui::TextUnformatted("No active scene");
			return;
		}
		ImGui::TextUnformatted("Hierarchy");
		ImGui::Separator();
		ImGui::BeginChild("##scene_hierarchy_widget_tree", ImVec2(0, 0.0f), true);
		DrawNode(*scene, "Scene", 0);
		ImGui::EndChild();
	}
} // namespace Engine
