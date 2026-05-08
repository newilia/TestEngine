#include "Engine/Editor/SceneHierarchyWidget.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/Utils.h"
#include "Engine/Editor/Editor.h"

#include <imgui.h>

#include <algorithm>
#include <unordered_set>

namespace Engine {
	std::shared_ptr<SceneNode> SceneHierarchyWidget::GetSelectedNode() const {
		for (const auto& weakNode : _selectionOrder) {
			if (auto node = weakNode.lock()) {
				return node;
			}
		}
		return nullptr;
	}

	std::vector<std::shared_ptr<SceneNode>> SceneHierarchyWidget::GetSelectedNodes() const {
		std::vector<std::shared_ptr<SceneNode>> result;
		result.reserve(_selectionOrder.size());
		for (const auto& weakNode : _selectionOrder) {
			if (auto node = weakNode.lock()) {
				result.push_back(std::move(node));
			}
		}
		return result;
	}

	bool SceneHierarchyWidget::ContainsNode(const SceneNode& node) const {
		const auto it = _selectionByRawPtr.find(std::addressof(node));
		if (it == _selectionByRawPtr.end()) {
			return false;
		}
		if (const auto locked = it->second.lock()) {
			return locked.get() == &node;
		}
		return false;
	}

	bool SceneHierarchyWidget::IsNodeSelected(const SceneNode& node) const {
		return ContainsNode(node);
	}

	void SceneHierarchyWidget::RebuildSelectionMapFromOrder() {
		_selectionByRawPtr.clear();
		_selectionByRawPtr.reserve(_selectionOrder.size());
		for (const auto& weakNode : _selectionOrder) {
			if (const auto locked = weakNode.lock()) {
				_selectionByRawPtr[locked.get()] = weakNode;
			}
		}
	}

	void SceneHierarchyWidget::RemoveFromSelectionOrder(const SceneNode& node) {
		const SceneNode* const target = std::addressof(node);
		_selectionOrder.erase(std::remove_if(_selectionOrder.begin(), _selectionOrder.end(),
		                          [target](const std::weak_ptr<SceneNode>& weakNode) {
			                          if (const auto locked = weakNode.lock()) {
				                          return locked.get() == target;
			                          }
			                          return false;
		                          }),
		    _selectionOrder.end());
	}

	void SceneHierarchyWidget::PruneExpiredSelection() {
		_selectionOrder.erase(std::remove_if(_selectionOrder.begin(), _selectionOrder.end(),
		                          [](const std::weak_ptr<SceneNode>& weakNode) {
			                          return weakNode.expired();
		                          }),
		    _selectionOrder.end());
		RebuildSelectionMapFromOrder();
		if (_selectionAnchor.expired()) {
			_selectionAnchor.reset();
		}
	}

	void SceneHierarchyWidget::ClearSelection() {
		_selectionOrder.clear();
		_selectionByRawPtr.clear();
		_selectionAnchor.reset();
		_scrollSelectionIntoViewPending = false;
	}

	void SceneHierarchyWidget::Select(std::shared_ptr<SceneNode> node) {
		PruneExpiredSelection();
		const SceneNode* const prevPtr = GetSelectedNode().get();
		const SceneNode* const nextPtr = node.get();
		if (prevPtr != nextPtr) {
			_scrollSelectionIntoViewPending = (nextPtr != nullptr);
		}
		_selectionOrder.clear();
		_selectionByRawPtr.clear();
		if (node) {
			_selectionAnchor = node;
			const auto* raw = static_cast<const SceneNode*>(node.get());
			_selectionByRawPtr.emplace(raw, node);
			_selectionOrder.push_back(std::move(node));
		}
		else {
			_selectionAnchor.reset();
		}
	}

	void SceneHierarchyWidget::ToggleSelection(std::shared_ptr<SceneNode> node) {
		PruneExpiredSelection();
		if (!node) {
			return;
		}
		const SceneNode* const target = node.get();
		if (const auto it = _selectionByRawPtr.find(target); it != _selectionByRawPtr.end()) {
			_selectionByRawPtr.erase(it);
			RemoveFromSelectionOrder(*target);
		}
		else {
			_selectionByRawPtr.emplace(static_cast<const SceneNode*>(target), node);
			_selectionOrder.push_back(node);
			_scrollSelectionIntoViewPending = true;
		}
		_selectionAnchor = std::move(node);
	}

	void SceneHierarchyWidget::AddToSelection(std::shared_ptr<SceneNode> node) {
		PruneExpiredSelection();
		if (!node || ContainsNode(*node)) {
			return;
		}
		_scrollSelectionIntoViewPending = true;
		_selectionAnchor = node;
		const auto* raw = static_cast<const SceneNode*>(node.get());
		_selectionByRawPtr.emplace(raw, node);
		_selectionOrder.push_back(std::move(node));
	}

	void SceneHierarchyWidget::SetSelection(std::vector<std::shared_ptr<SceneNode>> nodes) {
		_selectionOrder.clear();
		_selectionByRawPtr.clear();
		std::unordered_set<const SceneNode*> seen;
		seen.reserve(nodes.size());
		for (auto& node : nodes) {
			if (!node) {
				continue;
			}
			const SceneNode* const raw = static_cast<const SceneNode*>(node.get());
			if (seen.contains(raw)) {
				continue;
			}
			seen.insert(raw);
			_selectionByRawPtr.emplace(raw, node);
			_selectionOrder.push_back(std::move(node));
		}
		if (_selectionOrder.empty()) {
			_selectionAnchor.reset();
			_scrollSelectionIntoViewPending = false;
			return;
		}
		_selectionAnchor = _selectionOrder.back().lock();
		_scrollSelectionIntoViewPending = true;
	}

	void SceneHierarchyWidget::SelectRangeTo(
	    std::shared_ptr<SceneNode> targetNode, const std::vector<std::shared_ptr<SceneNode>>& treeOrder) {
		PruneExpiredSelection();
		if (!targetNode) {
			return;
		}
		auto anchor = _selectionAnchor.lock();
		if (!anchor) {
			Select(std::move(targetNode));
			return;
		}
		auto anchorIt = std::find(treeOrder.begin(), treeOrder.end(), anchor);
		auto targetIt = std::find(treeOrder.begin(), treeOrder.end(), targetNode);
		if (anchorIt == treeOrder.end() || targetIt == treeOrder.end()) {
			Select(std::move(targetNode));
			return;
		}
		auto [rangeBegin, rangeEnd] = std::minmax(anchorIt, targetIt);
		_selectionOrder.clear();
		_selectionByRawPtr.clear();
		for (auto it = rangeBegin; it != std::next(rangeEnd); ++it) {
			if (!*it) {
				continue;
			}
			const SceneNode* const raw = static_cast<const SceneNode*>(it->get());
			_selectionByRawPtr.emplace(raw, *it);
			_selectionOrder.push_back(*it);
		}
		_scrollSelectionIntoViewPending = true;
	}

	void SceneHierarchyWidget::BuildTreeOrder(
	    SceneNode& node, std::vector<std::shared_ptr<SceneNode>>& treeOrder) const {
		treeOrder.push_back(node.shared_from_this());
		for (const auto& child : node.GetChildren()) {
			if (child) {
				BuildTreeOrder(*child, treeOrder);
			}
		}
	}

	void SceneHierarchyWidget::DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth) {
		const std::string& name = node.GetName();
		const char* displayCStr = name.empty() ? emptyNamePlaceholder : name.c_str();
		const bool hasChildren = !node.GetChildren().empty();
		const bool isSelected = IsNodeSelected(node);
		void* const id = static_cast<void*>(std::addressof(node));

		ImGui::PushID(id);

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
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

		const bool isOpen = ImGui::TreeNodeEx("##hierarchy_node", nodeFlags, "%s", displayCStr);
		if (ImGui::IsItemClicked()) {
			auto clickedNode = node.shared_from_this();
			if (ImGui::GetIO().KeyShift) {
				std::vector<std::shared_ptr<SceneNode>> treeOrder;
				auto scene = Engine::MainContext::GetInstance().GetScene();
				auto root = scene ? scene->GetRoot() : nullptr;
				if (root) {
					BuildTreeOrder(*root, treeOrder);
				}
				SelectRangeTo(std::move(clickedNode), treeOrder);
			}
			else if (ImGui::GetIO().KeyCtrl) {
				ToggleSelection(std::move(clickedNode));
			}
			else {
				Select(std::move(clickedNode));
			}
		}

		if (ImGui::BeginPopupContextItem("hierarchy_node_ctx", ImGuiPopupFlags_MouseButtonRight)) {
			auto nodePtr = node.shared_from_this();
			Editor& editor = Editor::GetInstance();
			if (ImGui::MenuItem("Focus")) {
				MainContext::GetInstance().FocusCameraOnNode(nodePtr);
			}
			if (ImGui::MenuItem("Copy")) {
				(void)editor.CopyNode(nodePtr);
			}
			const bool canCut = nodePtr->GetParent() != nullptr;
			if (!canCut) {
				ImGui::BeginDisabled();
			}
			if (ImGui::MenuItem("Cut")) {
				(void)editor.CutNode(nodePtr);
			}
			if (ImGui::MenuItem("Delete")) {
				(void)editor.DeleteNode(nodePtr);
			}
			if (!canCut) {
				ImGui::EndDisabled();
			}
			const bool canPaste = editor.CanPasteClipboard();
			if (!canPaste) {
				ImGui::BeginDisabled();
			}
			if (ImGui::MenuItem("Paste")) {
				(void)editor.PasteClipboardOnto(nodePtr);
			}
			if (!canPaste) {
				ImGui::EndDisabled();
			}
			ImGui::EndPopup();
		}
		if (isSelected && _scrollSelectionIntoViewPending) {
			ImGui::SetScrollHereY(0.5f);
			_scrollSelectionIntoViewPending = false;
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

		ImGui::PopID();
	}

	void SceneHierarchyWidget::Draw(const std::shared_ptr<Scene>& scene) {
		PruneExpiredSelection();
		if (!scene) {
			ClearSelection();
			ImGui::TextUnformatted("No active scene");
			return;
		}
		const auto root = scene->GetRoot();
		if (!root) {
			ClearSelection();
			ImGui::TextUnformatted("No active scene");
			return;
		}
		ImGui::TextUnformatted("Scene hierarchy");
		ImGui::Separator();
		ImGui::BeginChild("##scene_hierarchy_widget_tree", ImVec2(0, 0.0f), true);
		DrawNode(*root, "Root", 0);
		ImGui::EndChild();
	}
} // namespace Engine
