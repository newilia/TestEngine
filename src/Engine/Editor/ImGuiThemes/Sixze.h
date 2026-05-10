#pragma once

#include <imgui.h>

namespace Editor::Themes::Sixze {

	void ApplyStyle();

	ImVec4 AdjustHueAndSaturation(const ImVec4& Color, const float HueShift, const float SaturationScale);

} // namespace Editor::Themes::Sixze
