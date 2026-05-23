#include "Engine/Editor/SceneHierarchyWidget.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Editor/Commands/EditorSceneHelpers.h"
#include "Engine/Editor/Editor.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <utility>

namespace {
	constexpr const char* kHierarchyNodeDragPayloadType = "TE_SCENE_HIERARCHY_NODE";

	int HierarchyRenameInputCallback(ImGuiInputTextCallbackData* data) {
		if (data->EventFlag != ImGuiInputTextFlags_CallbackAlways) {
			return 0;
		}
		auto* const placeCursorAtEnd = static_cast<bool*>(data->UserData);
		if (placeCursorAtEnd && *placeCursorAtEnd) {
			data->CursorPos = data->BufTextLen;
			data->SelectionStart = data->SelectionEnd = data->BufTextLen;
			*placeCursorAtEnd = false;
		}
		return 0;
	}
} // namespace

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
		if (_renamingNode.expired()) {
			CancelRenaming();
		}
	}

	void SceneHierarchyWidget::ClearSelection() {
		_selectionOrder.clear();
		_selectionByRawPtr.clear();
		_selectionAnchor.reset();
		_scrollSelectionIntoViewPending = false;
		CancelRenaming();
	}

	bool SceneHierarchyWidget::IsRenaming(const SceneNode& node) const {
		const auto renaming = _renamingNode.lock();
		return renaming && renaming.get() == &node;
	}

	void SceneHierarchyWidget::StartRenaming(SceneNode& node) {
		_renamingNode = node.shared_from_this();
		_renameEditBuffer.assign(kRenameBufferSize, '\0');
		const std::string& name = node.GetName();
		const std::size_t copyLen = std::min(name.size(), kRenameBufferSize - 1);
		if (copyLen > 0) {
			std::memcpy(_renameEditBuffer.data(), name.data(), copyLen);
		}
		_renameFocusNextFrame = true;
		_renamePlaceCursorAtEnd = true;
	}

	void SceneHierarchyWidget::CancelRenaming() {
		_renamingNode.reset();
		_renameEditBuffer.clear();
		_renameFocusNextFrame = false;
		_renamePlaceCursorAtEnd = false;
	}

	void SceneHierarchyWidget::CommitRenaming(SceneNode& node) {
		const auto nodePtr = _renamingNode.lock();
		if (!nodePtr || nodePtr.get() != &node) {
			CancelRenaming();
			return;
		}
		const std::string newName(_renameEditBuffer.data());
		CancelRenaming();
		(void)Editor::GetInstance().RenameNode(nodePtr, newName);
	}

	void SceneHierarchyWidget::Select(std::shared_ptr<SceneNode> node) {
		PruneExpiredSelection();
		_scrollSelectionIntoViewPending = (node != nullptr);
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

	bool SceneHierarchyWidget::IsOnPathToRevealSelectionTarget(const SceneNode& node) const {
		if (IsNodeSelected(node)) {
			return true;
		}
		for (const auto& weakNode : _selectionOrder) {
			const auto selected = weakNode.lock();
			if (!selected) {
				continue;
			}
			for (auto ancestor = selected->GetParent(); ancestor != nullptr; ancestor = ancestor->GetParent()) {
				if (ancestor.get() == &node) {
					return true;
				}
			}
		}
		return false;
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

	std::vector<std::shared_ptr<SceneNode>> SceneHierarchyWidget::ResolveDraggedNodes(SceneNode& source) const {
		std::vector<std::shared_ptr<SceneNode>> treeOrder;
		if (const auto scene = MainContext::GetInstance().GetScene()) {
			if (const auto root = scene->GetRoot()) {
				BuildTreeOrder(*root, treeOrder);
			}
		}

		std::vector<std::shared_ptr<SceneNode>> result;
		if (IsNodeSelected(source)) {
			std::unordered_set<const SceneNode*> selectedSet;
			for (const auto& selected : GetSelectedNodes()) {
				if (selected) {
					selectedSet.insert(selected.get());
				}
			}
			for (const auto& ordered : treeOrder) {
				if (ordered && ordered->GetParent() != nullptr && selectedSet.contains(ordered.get())) {
					result.push_back(ordered);
				}
			}
		}
		else if (source.GetParent() != nullptr) {
			result.push_back(source.shared_from_this());
		}
		return result;
	}

	SceneHierarchyWidget::HierarchyDropHint SceneHierarchyWidget::ReadDropHintFromItem() {
		const ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		const float height = rect.GetHeight();
		const float y = ImGui::GetMousePos().y;
		if (height <= 0.f) {
			return HierarchyDropHint::Into;
		}
		const float topBand = rect.Min.y + height * 0.25f;
		const float bottomBand = rect.Max.y - height * 0.25f;
		if (y < topBand) {
			return HierarchyDropHint::Before;
		}
		if (y > bottomBand) {
			return HierarchyDropHint::After;
		}
		return HierarchyDropHint::Into;
	}

	void SceneHierarchyWidget::DrawDropHintIndicator(const HierarchyDropHint hint) {
		const ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		ImDrawList* const drawList = ImGui::GetForegroundDrawList();
		const ImU32 color = ImGui::GetColorU32(ImGuiCol_DragDropTarget);
		if (hint == HierarchyDropHint::Before) {
			drawList->AddLine(ImVec2(rect.Min.x, rect.Min.y), ImVec2(rect.Max.x, rect.Min.y), color, 2.f);
		}
		else if (hint == HierarchyDropHint::After) {
			drawList->AddLine(ImVec2(rect.Min.x, rect.Max.y), ImVec2(rect.Max.x, rect.Max.y), color, 2.f);
		}
	}

	void SceneHierarchyWidget::DrawDropIntoHighlightForItem() {
		const ImRect itemRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		ImDrawList* const drawList = ImGui::GetForegroundDrawList();
		const ImU32 fillColor = ImGui::GetColorU32(ImGuiCol_DragDropTarget, 0.35f);
		const ImU32 borderColor = ImGui::GetColorU32(ImGuiCol_DragDropTarget);
		drawList->AddRectFilled(itemRect.Min, itemRect.Max, fillColor);
		drawList->AddRect(itemRect.Min, itemRect.Max, borderColor, 0.f, 0, 2.f);
	}

	std::optional<std::pair<std::shared_ptr<SceneNode>, std::size_t>> SceneHierarchyWidget::TryResolveDropTarget(
	    const HierarchyDropHint hint, SceneNode& target, const std::vector<std::shared_ptr<SceneNode>>& draggedNodes) {
		if (draggedNodes.empty()) {
			return std::nullopt;
		}

		const auto targetPtr = target.shared_from_this();
		for (const auto& dragged : draggedNodes) {
			if (!dragged) {
				return std::nullopt;
			}
			if (dragged.get() == &target || EditorCommands::IsNodeInSubtree(targetPtr, dragged)) {
				return std::nullopt;
			}
		}

		switch (hint) {
		case HierarchyDropHint::Before: {
			const auto parent = target.GetParent();
			if (!parent) {
				return std::nullopt;
			}
			const auto& children = parent->GetChildren();
			const auto it = std::find(children.begin(), children.end(), targetPtr);
			if (it == children.end()) {
				return std::nullopt;
			}
			return std::pair{parent, static_cast<std::size_t>(std::distance(children.begin(), it))};
		}
		case HierarchyDropHint::After: {
			const auto parent = target.GetParent();
			if (!parent) {
				return std::nullopt;
			}
			const auto& children = parent->GetChildren();
			const auto it = std::find(children.begin(), children.end(), targetPtr);
			if (it == children.end()) {
				return std::nullopt;
			}
			return std::pair{parent, static_cast<std::size_t>(std::distance(children.begin(), it)) + 1};
		}
		case HierarchyDropHint::Into:
			return std::pair{targetPtr, target.GetChildren().size()};
		}
		return std::nullopt;
	}

	void SceneHierarchyWidget::HandleHierarchyDrop(SceneNode& target, SceneNode& dragSource) {
		const auto draggedNodes = ResolveDraggedNodes(dragSource);
		const auto hint = ReadDropHintFromItem();
		const auto dropTarget = TryResolveDropTarget(hint, target, draggedNodes);
		if (!dropTarget) {
			return;
		}
		(void)Editor::GetInstance().MoveNodesInHierarchy(draggedNodes, dropTarget->first, dropTarget->second);
	}

	void SceneHierarchyWidget::DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth) {
		const std::string& name = node.GetName();
		const char* displayCStr = name.empty() ? emptyNamePlaceholder : name.c_str();
		const bool hasChildren = !node.GetChildren().empty();
		const bool isSelected = IsNodeSelected(node);
		const bool isRenaming = IsRenaming(node);
		void* const id = static_cast<void*>(std::addressof(node));

		ImGui::PushID(id);

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
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

		if (_scrollSelectionIntoViewPending && hasChildren && IsOnPathToRevealSelectionTarget(node)) {
			ImGui::SetNextItemOpen(true, ImGuiCond_Always);
		}

		const bool isOpen = isRenaming ? ImGui::TreeNodeEx("##hierarchy_node", nodeFlags, "")
		                               : ImGui::TreeNodeEx("##hierarchy_node", nodeFlags, "%s", displayCStr);

		if (isRenaming) {
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.f);
			if (_renameFocusNextFrame) {
				ImGui::SetKeyboardFocusHere();
				_renameFocusNextFrame = false;
			}
			const ImGuiInputTextFlags renameFlags =
			    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways;
			(void)ImGui::InputText("##hierarchy_rename", _renameEditBuffer.data(), _renameEditBuffer.size(),
			    renameFlags, HierarchyRenameInputCallback, &_renamePlaceCursorAtEnd);
			if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
				CancelRenaming();
			}
			else if (ImGui::IsItemDeactivated()) {
				CommitRenaming(node);
			}
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			StartRenaming(node);
		}

		const bool canDrag = node.GetParent() != nullptr && !isRenaming;
		if (canDrag && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			SceneNode* const dragSource = &node;
			ImGui::SetDragDropPayload(kHierarchyNodeDragPayloadType, &dragSource, sizeof(dragSource));
			const auto draggedNodes = ResolveDraggedNodes(node);
			if (draggedNodes.size() > 1) {
				ImGui::TextUnformatted("Move nodes");
				ImGui::Text("%zu nodes", draggedNodes.size());
			}
			else {
				ImGui::Text("Move %s", displayCStr);
			}
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget()) {
			constexpr ImGuiDragDropFlags acceptFlags =
			    ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
			if (const ImGuiPayload* const payload =
			        ImGui::AcceptDragDropPayload(kHierarchyNodeDragPayloadType, acceptFlags)) {
				if (payload->DataSize == sizeof(SceneNode*)) {
					auto* const dragSource = *static_cast<SceneNode* const*>(payload->Data);
					if (dragSource != nullptr) {
						const auto hint = ReadDropHintFromItem();
						const auto draggedNodes = ResolveDraggedNodes(*dragSource);
						if (TryResolveDropTarget(hint, node, draggedNodes)) {
							if (hint == HierarchyDropHint::Into) {
								DrawDropIntoHighlightForItem();
							}
							else {
								DrawDropHintIndicator(hint);
							}
						}
						if (payload->IsDelivery()) {
							HandleHierarchyDrop(node, *dragSource);
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (!isRenaming && ImGui::IsItemClicked()) {
			const ImGuiIO& io = ImGui::GetIO();
			constexpr float kClickDragThresholdSqr = 4.f;
			if (io.MouseDragMaxDistanceSqr[0] <= kClickDragThresholdSqr) {
				auto clickedNode = node.shared_from_this();
				if (io.KeyShift) {
					std::vector<std::shared_ptr<SceneNode>> treeOrder;
					auto scene = Engine::MainContext::GetInstance().GetScene();
					auto root = scene ? scene->GetRoot() : nullptr;
					if (root) {
						BuildTreeOrder(*root, treeOrder);
					}
					SelectRangeTo(std::move(clickedNode), treeOrder);
				}
				else if (io.KeyCtrl) {
					ToggleSelection(std::move(clickedNode));
				}
				else if (!isSelected) {
					Select(std::move(clickedNode));
				}
				else {
					_selectionAnchor = clickedNode;
				}
			}
		}

		if (ImGui::BeginPopupContextItem("hierarchy_node_ctx", ImGuiPopupFlags_MouseButtonRight)) {
			auto nodePtr = node.shared_from_this();
			Editor& editor = Editor::GetInstance();
			if (ImGui::MenuItem("Rename")) {
				StartRenaming(node);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Add child")) {
				(void)editor.AddEmptyChildNode(nodePtr);
			}
			const bool canAddSibling = nodePtr->GetParent() != nullptr;
			if (!canAddSibling) {
				ImGui::BeginDisabled();
			}
			if (ImGui::MenuItem("Add sibling")) {
				(void)editor.AddEmptySiblingNode(nodePtr);
			}
			if (!canAddSibling) {
				ImGui::EndDisabled();
			}
			ImGui::Separator();
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
			ImGui::Separator();
			if (ImGui::MenuItem("Save as scene object…")) {
				(void)editor.SaveNodeAsSceneObject(nodePtr);
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
		ImGui::BeginChild("##scene_hierarchy_widget_tree", ImVec2(0, 0.0f), true);
		DrawNode(*root, "<unnamed>", 0);
		ImGui::EndChild();
	}
} // namespace Engine
