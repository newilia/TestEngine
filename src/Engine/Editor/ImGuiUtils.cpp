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

	bool ShouldAllowCameraWheelZoom() {
		const ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse) {
			return true;
		}

		// Menu bar dropdowns (and other non-modal popups) set WantCaptureMouse via
		// has_open_popup, but not WantCaptureMouseUnlessPopupClose — see imgui.cpp
		// UpdateHoveredWindowAndCaptureFlags().
		if (!io.WantCaptureMouseUnlessPopupClose) {
			return true;
		}

		// Fallback: any open popup (menus, combos, context menus) should not block
		// scene zoom; docked panels still use WantCaptureMouseUnlessPopupClose above.
		if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) {
			return true;
		}

		if (ImGuiWindow* hovered = ImGui::GetCurrentContext()->HoveredWindow) {
			if ((hovered->Flags & ImGuiWindowFlags_ChildMenu) != 0) {
				return true;
			}
		}

		return false;
	}
} // namespace ImGuiUtils
