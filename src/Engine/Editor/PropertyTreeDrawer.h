#pragma once

#include "Engine/Core/PropertyTree.h"

#include <memory>

class Scene;

namespace Engine {

	struct PropertyTreeDrawOptions
	{
		/// When the tree is a single `PropertyKind::Object` root (typical `pushObject` / codegen wrapper),
		/// draw its children directly so an outer UI header (e.g. inspector `CollapsingHeader`) is not duplicated.
		bool unwrapSingleRootObject = false;
		/// If non-null, set to true when a leaf with `PropertyMeta::hasMixedValues` was edited (merged inspector cache).
		bool* anyLeafEdited = nullptr;
		/// If set, `PropertyKind::SceneRef` picker uses this scene; otherwise `MainContext::GetScene()`.
		std::shared_ptr<Scene> sceneForSceneRefsOverride;
	};

	/// ImGui rendering for `PropertyTree` built from `IInspectable::BuildPropertyTree`.
	class PropertyTreeDrawer
	{
	public:
		void Draw(const PropertyTree& tree, PropertyTreeDrawOptions options = {}) const;

	private:
		void DrawNode(const PropertyNode& node, PropertyTreeDrawOptions drawOptions) const;
	};

} // namespace Engine
