#include "Engine/Editor/EditorVisualTheme.h"

#include <imgui.h>

#include <algorithm>

namespace Engine::EditorVisualTheme {
	namespace {

		ImVec4 BaseHeaderColor(InspectorSectionHeaderStyle section) {
			switch (section) {
			case InspectorSectionHeaderStyle::SceneNode:
				return kInspectorSectionHeaderSceneNode;
			case InspectorSectionHeaderStyle::Transform:
				return kInspectorSectionHeaderTransform;
			case InspectorSectionHeaderStyle::SortingStrategy:
				return kInspectorSectionHeaderSortingStrategy;
			case InspectorSectionHeaderStyle::Visual:
				return kInspectorSectionHeaderVisual;
			case InspectorSectionHeaderStyle::Behaviour:
				return kInspectorSectionHeaderBehaviour;
			}
			return kInspectorSectionHeaderFallback;
		}

		ImVec4 AdjustRgb(const ImVec4& c, float delta) {
			return ImVec4(std::clamp(c.x + delta, 0.f, 1.f), std::clamp(c.y + delta, 0.f, 1.f),
			    std::clamp(c.z + delta, 0.f, 1.f), c.w);
		}

	} // namespace

	void PushInspectorSectionHeaderColors(InspectorSectionHeaderStyle section) {
		const ImVec4 base = BaseHeaderColor(section);
		ImGui::PushStyleColor(ImGuiCol_Header, base);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, AdjustRgb(base, 0.12f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, AdjustRgb(base, -0.08f));
	}

	void PopInspectorSectionHeaderColors() {
		ImGui::PopStyleColor(3);
	}

} // namespace Engine::EditorVisualTheme
