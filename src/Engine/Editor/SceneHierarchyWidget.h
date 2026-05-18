#pragma once

#include "Engine/Core/Scene.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

class SceneNode;

namespace Engine {

	/// ImGui widget: tree of `SceneNode` and selection.
	class SceneHierarchyWidget
	{
	public:
		std::shared_ptr<SceneNode> GetSelectedNode() const;
		std::vector<std::shared_ptr<SceneNode>> GetSelectedNodes() const;
		bool IsNodeSelected(const SceneNode& node) const;
		void ClearSelection();
		void Select(std::shared_ptr<SceneNode> node);
		void ToggleSelection(std::shared_ptr<SceneNode> node);
		void AddToSelection(std::shared_ptr<SceneNode> node);
		void SelectRangeTo(
		    std::shared_ptr<SceneNode> targetNode, const std::vector<std::shared_ptr<SceneNode>>& treeOrder);
		void SetSelection(std::vector<std::shared_ptr<SceneNode>> nodes);
		/// Renders the hierarchy and optional "no scene" state; may clear selection when `scene` is null.
		void Draw(const std::shared_ptr<Scene>& scene);

	private:
		enum class HierarchyDropHint
		{
			Before,
			Into,
			After
		};

		void PruneExpiredSelection();
		bool ContainsNode(const SceneNode& node) const;
		void DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth);
		void BuildTreeOrder(SceneNode& node, std::vector<std::shared_ptr<SceneNode>>& treeOrder) const;
		void RemoveFromSelectionOrder(const SceneNode& node);
		void RebuildSelectionMapFromOrder();

		[[nodiscard]] std::vector<std::shared_ptr<SceneNode>> ResolveDraggedNodes(SceneNode& source) const;
		[[nodiscard]] static HierarchyDropHint ReadDropHintFromItem();
		static void DrawDropHintIndicator(HierarchyDropHint hint);
		static void DrawDropIntoHighlightForItem();
		[[nodiscard]] static std::optional<std::pair<std::shared_ptr<SceneNode>, std::size_t>> TryResolveDropTarget(
		    HierarchyDropHint hint, SceneNode& target, const std::vector<std::shared_ptr<SceneNode>>& draggedNodes);
		void HandleHierarchyDrop(SceneNode& target, SceneNode& dragSource);

		std::vector<std::weak_ptr<SceneNode>> _selectionOrder;
		std::unordered_map<const SceneNode*, std::weak_ptr<SceneNode>> _selectionByRawPtr;
		std::weak_ptr<SceneNode> _selectionAnchor;
		bool _scrollSelectionIntoViewPending = false;
	};

} // namespace Engine
