#pragma once

#include "Engine/Core/Scene.h"

#include <memory>

namespace Engine {

	/// ImGui widget: read-only view of a selected `SceneNode` (name, parent, transform, ...).
	class NodeInspectorWidget
	{
	public:
		/// Renders the inspector. Pass null when nothing is selected.
		void Draw(const std::shared_ptr<SceneNode>& node) const;
	};

} // namespace Engine
