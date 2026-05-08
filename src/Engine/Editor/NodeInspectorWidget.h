#pragma once

#include "Engine/Core/Scene.h"
#include "Engine/Editor/PropertyTreeDrawer.h"

#include <memory>
#include <vector>

namespace Engine {

	/// ImGui widget: read-only view of a selected `SceneNode` (name, parent, transform, ...).
	class NodeInspectorWidget
	{
	public:
		/// Renders the inspector for current multi-selection.
		void Draw(const std::vector<std::shared_ptr<SceneNode>>& nodes) const;

	private:
		PropertyTreeDrawer _propertyDrawer{};
	};

} // namespace Engine
