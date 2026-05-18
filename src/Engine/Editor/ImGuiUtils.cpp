#include "Engine/Editor/ImGuiUtils.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace ImGuiUtils {
	float GetRemainingWidth() {
		auto& g = *ImGui::GetCurrentContext();
		auto width = g.CurrentWindow->DC.ItemWidthDefault;
		auto cursorPos = ImGui::GetCursorPos().x;
		return width - cursorPos + 10;
	}
} // namespace ImGuiUtils
