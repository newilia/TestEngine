#include "Engine/Editor/EditorVisualTheme.h"

#include <imgui.h>

#include <algorithm>

namespace Engine::EditorVisualTheme {
	namespace {

		ImVec4 BaseHeaderColor(InspectorSectionHeaderStyle section) {
			switch (section) {
			case InspectorSectionHeaderStyle::SceneNode:
				return ImVec4(0.22f, 0.38f, 0.58f, 1.f);
			case InspectorSectionHeaderStyle::Transform:
				return ImVec4(0.24f, 0.52f, 0.36f, 1.f);
			case InspectorSectionHeaderStyle::SortingStrategy:
				return ImVec4(0.55f, 0.40f, 0.22f, 1.f);
			case InspectorSectionHeaderStyle::Visual:
				return ImVec4(0.48f, 0.28f, 0.55f, 1.f);
			case InspectorSectionHeaderStyle::Behaviour:
				return ImVec4(0.30f, 0.42f, 0.48f, 1.f);
			}
			return ImVec4(0.26f, 0.26f, 0.28f, 1.f);
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
