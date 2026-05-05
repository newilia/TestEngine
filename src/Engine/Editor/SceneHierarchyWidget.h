#pragma once

#include "Engine/Core/Scene.h"

#include <memory>

namespace Engine {

	/// ImGui widget: tree of `SceneNode` and selection.
	class SceneHierarchyWidget
	{
	public:
		std::shared_ptr<SceneNode> GetSelectedNode() const;
		void ClearSelection();
		void Select(std::shared_ptr<SceneNode> node);
		/// Renders the hierarchy and optional "no scene" state; may clear selection when `scene` is null.
		void Draw(const std::shared_ptr<Scene>& scene);

	private:
		void DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth);

	private:
		std::weak_ptr<SceneNode> _selectedNode;
		bool _scrollSelectionIntoViewPending = false;
	};

} // namespace Engine
