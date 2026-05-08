#pragma once

#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/Scene.h"
#include "Engine/Editor/PropertyTreeDrawer.h"

#include <optional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Engine {

	/// Holds merged inspector property trees across frames for stable multi-selection.
	struct NodeInspectorMergeState
	{
		std::vector<const SceneNode*> fingerprint;
		std::optional<PropertyTree> sceneNodeMerged;
		std::optional<PropertyTree> transformMerged;
		std::optional<PropertyTree> sortingMerged;
		std::optional<PropertyTree> visualMerged;
		std::unordered_map<std::type_index, std::optional<PropertyTree>> behaviourMerged;
		std::vector<std::pair<std::type_index, std::string>> commonBehaviourTitles;
	};

	/// ImGui widget: read-only view of a selected `SceneNode` (name, parent, transform, ...).
	class NodeInspectorWidget
	{
	public:
		/// Renders the inspector for current multi-selection.
		void Draw(const std::vector<std::shared_ptr<SceneNode>>& nodes) const;

	private:
		void DrawSingleNode(const std::shared_ptr<SceneNode>& node) const;
		void DrawMultiNode(const std::vector<std::shared_ptr<SceneNode>>& nodes) const;

		PropertyTreeDrawer _propertyDrawer{};
		mutable NodeInspectorMergeState _mergeState{};
	};

} // namespace Engine
