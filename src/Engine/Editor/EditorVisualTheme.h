#pragma once

#include <SFML/Graphics/Color.hpp>

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

	void PushInspectorSectionHeaderColors(InspectorSectionHeaderStyle section);
	void PopInspectorSectionHeaderColors();

} // namespace Engine::EditorVisualTheme
