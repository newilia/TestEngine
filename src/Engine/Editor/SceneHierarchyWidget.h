#pragma once

#include "Engine/Core/Scene.h"

#include <memory>

namespace Engine {

	/// ImGui widget: tree of `SceneNode` and selection.
	class SceneHierarchyWidget
	{
	public:
		[[nodiscard]] std::shared_ptr<SceneNode> GetSelected() const;
		void ClearSelection();
		/// Renders the hierarchy and optional "no scene" state; may clear selection when `scene` is null.
		void Draw(const std::shared_ptr<Scene>& scene);

	private:
		void DrawNode(SceneNode& node, const char* emptyNamePlaceholder, int depth);

		std::weak_ptr<SceneNode> _selectedNode;
	};

} // namespace Engine
