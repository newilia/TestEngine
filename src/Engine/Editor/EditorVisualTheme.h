#pragma once

#include <SFML/Graphics/Color.hpp>

#include <imgui.h>

#include <cstdint>

namespace Engine::EditorVisualTheme {

	inline constexpr float kHierarchySelectionOutlinePadPx = 3.f;
	inline constexpr float kHierarchySelectionOutlineThickness = 2.f;
	inline constexpr float kHierarchySelectionFallbackHalfSize = 6.f;

	inline const sf::Color kHierarchySelectionOutlineColor{120u, 190u, 255u, 220u};
	inline const sf::Color kHierarchySelectionChildOutlineColor{255u, 200u, 120u, 210u};

	enum class InspectorSectionHeaderStyle : std::uint8_t
	{
		SceneNode,
		Transform,
		SortingStrategy,
		Visual,
		Behaviour,
	};

	inline constexpr ImVec4 kInspectorSectionHeaderSceneNode{0.22f, 0.38f, 0.58f, 1.f};
	inline constexpr ImVec4 kInspectorSectionHeaderTransform{0.24f, 0.52f, 0.36f, 1.f};
	inline constexpr ImVec4 kInspectorSectionHeaderSortingStrategy{0.55f, 0.40f, 0.22f, 1.f};
	inline constexpr ImVec4 kInspectorSectionHeaderVisual{0.48f, 0.28f, 0.55f, 1.f};
	inline constexpr ImVec4 kInspectorSectionHeaderBehaviour{0.30f, 0.42f, 0.48f, 1.f};
	inline constexpr ImVec4 kInspectorSectionHeaderFallback{0.26f, 0.26f, 0.28f, 1.f};

	void PushInspectorSectionHeaderColors(InspectorSectionHeaderStyle section);
	void PopInspectorSectionHeaderColors();

} // namespace Engine::EditorVisualTheme
