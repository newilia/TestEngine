#pragma once

#include "Engine/Core/PropertyTree.h"

namespace Engine {

	/// ImGui rendering for `PropertyTree` built from `IInspectable::BuildPropertyTree`.
	class PropertyTreeDrawer
	{
	public:
		void Draw(const PropertyTree& tree) const;

	private:
		void drawNode(const PropertyNode& node) const;
	};

} // namespace Engine
