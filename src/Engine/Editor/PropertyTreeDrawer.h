#pragma once

#include "Engine/Core/PropertyTree.h"

namespace Engine {

	struct PropertyTreeDrawOptions
	{
		/// When the tree is a single `PropertyKind::Object` root (typical `pushObject` / codegen wrapper),
		/// draw its children directly so an outer UI header (e.g. inspector `CollapsingHeader`) is not duplicated.
		bool unwrapSingleRootObject = false;
	};

	/// ImGui rendering for `PropertyTree` built from `IInspectable::BuildPropertyTree`.
	class PropertyTreeDrawer
	{
	public:
		void Draw(const PropertyTree& tree, PropertyTreeDrawOptions options = {}) const;

	private:
		void DrawNode(const PropertyNode& node) const;
	};

} // namespace Engine
