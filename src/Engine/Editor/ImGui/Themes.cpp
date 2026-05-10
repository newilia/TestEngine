#include "Themes.h"

#include <SFML/Graphics/Color.hpp>

#include <imgui.h>

// Original source: https://github.com/ocornut/imgui/issues/707#issuecomment-4121907946

namespace Editor::Themes {

	void SetupItamagoStyle(bool bStyleDark_, float alpha_) {
		ImGuiStyle& style = ImGui::GetStyle();

		// light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
		style.Alpha = 1.0f;
		style.FrameRounding = 3.0f;
		style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
		//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		//style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		//style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		//style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		//style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		//style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
		//style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		//style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

		if (bStyleDark_) {
			for (int i = 0; i <= ImGuiCol_COUNT; i++) {
				ImVec4& col = style.Colors[i];
				float H, S, V;
				ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

				if (S < 0.1f) {
					V = 1.0f - V;
				}
				ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
				if (col.w < 1.00f) {
					col.w *= alpha_;
				}
			}
		}
		else {
			for (int i = 0; i <= ImGuiCol_COUNT; i++) {
				ImVec4& col = style.Colors[i];
				if (col.w < 1.00f) {
					col.x *= alpha_;
					col.y *= alpha_;
					col.z *= alpha_;
					col.w *= alpha_;
				}
			}
		}
	}

	void SetupSixzeStyle() {
		ImGuiStyle& Style = ImGui::GetStyle();

		const float Hue = 0.0f;              // [0,1] range.
		const float Saturation = 1.0f;       // [0,6] range.
		const float SaturationAccent = 1.0f; // [0,6] range.
		const float Transparency = 0.65f;
		const float BorderSize = 0.0f;

		Style.FrameBorderSize = BorderSize;
		Style.ImageBorderSize = BorderSize;
		Style.TabBorderSize = BorderSize;
		Style.TabBarBorderSize = 3.0f;
		Style.WindowRounding = 4.0f;
		Style.ChildRounding = 4.0f;
		Style.FrameRounding = 4.0f;
		Style.GrabRounding = 4.0f;
		Style.TabRounding = 4.0f;

		ImVec4 TextColor{1.000f, 1.000f, 1.000f, 1.000f};
		ImVec4 TextDimmedColor{0.357f, 0.482f, 0.549f, 1.000f};
		ImVec4 BackroundColor{0.110f, 0.149f, 0.169f, 1.000f};
		ImVec4 BackroundChildColor{0.090f, 0.122f, 0.141f, 1.000f};
		ImVec4 BackroundDimmedColor{0.000f, 0.000f, 0.000f, 0.600f};
		ImVec4 TitleColor{0.078f, 0.102f, 0.122f, 1.000f};
		ImVec4 HeaderColor{0.184f, 0.247f, 0.286f, 1.000f};
		ImVec4 Accent1Color{0.000f, 0.490f, 1.000f, 1.000f};
		ImVec4 Accent2Color{0.000f, 0.412f, 0.824f, 1.000f};
		ImVec4 Accent1AlternativeColor{0.302f, 0.408f, 0.475f, 1.000f};
		ImVec4 Accent2AlternativeColor{0.251f, 0.337f, 0.392f, 1.000f};
		ImVec4 TransparentColor{0.0f, 0.0f, 0.0f, 0.0f};

		auto AdjustHueAndSaturation = [](const ImVec4& Color, const float HueShift,
		                                  const float SaturationScale) -> ImVec4 {
			ImVec4 ResultColor = Color;
			ImGui::ColorConvertRGBtoHSV(
			    ResultColor.x, ResultColor.y, ResultColor.z, ResultColor.x, ResultColor.y, ResultColor.z);

			ResultColor.x += HueShift;
			ResultColor.y *= SaturationScale;

			ImGui::ColorConvertHSVtoRGB(
			    ResultColor.x, ResultColor.y, ResultColor.z, ResultColor.x, ResultColor.y, ResultColor.z);
			return ResultColor;
		};

		TextColor = AdjustHueAndSaturation(TextColor, Hue, Saturation);
		TextDimmedColor = AdjustHueAndSaturation(TextDimmedColor, Hue, Saturation);
		BackroundColor = AdjustHueAndSaturation(BackroundColor, Hue, Saturation);
		BackroundChildColor = AdjustHueAndSaturation(BackroundChildColor, Hue, Saturation);
		BackroundDimmedColor = AdjustHueAndSaturation(BackroundDimmedColor, Hue, Saturation);
		TitleColor = AdjustHueAndSaturation(TitleColor, Hue, Saturation);
		HeaderColor = AdjustHueAndSaturation(HeaderColor, Hue, Saturation);
		Accent1Color = AdjustHueAndSaturation(Accent1Color, Hue, SaturationAccent);
		Accent2Color = AdjustHueAndSaturation(Accent2Color, Hue, SaturationAccent);
		Accent1AlternativeColor = AdjustHueAndSaturation(Accent1AlternativeColor, Hue, SaturationAccent);
		Accent2AlternativeColor = AdjustHueAndSaturation(Accent2AlternativeColor, Hue, SaturationAccent);

		ImVec4 BackroundTransparentColor = BackroundColor;
		BackroundTransparentColor.w *= Transparency;

		ImVec4 BackroundChildTransparentColor = BackroundChildColor;
		BackroundChildTransparentColor.w *= Transparency;

		ImVec4 TitleTransparentColor = TitleColor;
		TitleTransparentColor.w *= Transparency;

		ImVec4 HeaderTransparentColor = HeaderColor;
		HeaderTransparentColor.w *= Transparency;

		Style.Colors[ImGuiCol_Text] = TextColor;
		Style.Colors[ImGuiCol_TextDisabled] = TextDimmedColor;
		Style.Colors[ImGuiCol_WindowBg] = BackroundTransparentColor;
		Style.Colors[ImGuiCol_ChildBg] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_PopupBg] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_Border] = TitleColor;
		Style.Colors[ImGuiCol_BorderShadow] = TransparentColor;
		Style.Colors[ImGuiCol_FrameBg] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_FrameBgHovered] = Accent1AlternativeColor;
		Style.Colors[ImGuiCol_FrameBgActive] = Accent2AlternativeColor;
		Style.Colors[ImGuiCol_TitleBg] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_TitleBgActive] = TitleTransparentColor;
		Style.Colors[ImGuiCol_TitleBgCollapsed] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_MenuBarBg] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_ScrollbarBg] = TitleTransparentColor;
		Style.Colors[ImGuiCol_ScrollbarGrab] = HeaderColor;
		Style.Colors[ImGuiCol_ScrollbarGrabHovered] = Accent1AlternativeColor;
		Style.Colors[ImGuiCol_ScrollbarGrabActive] = Accent2AlternativeColor;
		Style.Colors[ImGuiCol_CheckMark] = Accent1Color;
		Style.Colors[ImGuiCol_SliderGrab] = Accent1Color;
		Style.Colors[ImGuiCol_SliderGrabActive] = Accent2Color;
		Style.Colors[ImGuiCol_Button] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_ButtonHovered] = Accent1Color;
		Style.Colors[ImGuiCol_ButtonActive] = Accent2Color;
		Style.Colors[ImGuiCol_Header] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_HeaderHovered] = Accent1Color;
		Style.Colors[ImGuiCol_HeaderActive] = Accent2Color;
		Style.Colors[ImGuiCol_Separator] = HeaderColor;
		Style.Colors[ImGuiCol_SeparatorHovered] = Accent1Color;
		Style.Colors[ImGuiCol_SeparatorActive] = Accent2Color;
		Style.Colors[ImGuiCol_ResizeGrip] = HeaderColor;
		Style.Colors[ImGuiCol_ResizeGripHovered] = Accent1Color;
		Style.Colors[ImGuiCol_ResizeGripActive] = Accent2Color;
		Style.Colors[ImGuiCol_InputTextCursor] = TextColor;
		Style.Colors[ImGuiCol_TabHovered] = Accent1Color;
		Style.Colors[ImGuiCol_Tab] = TransparentColor;
		Style.Colors[ImGuiCol_TabSelected] = HeaderTransparentColor;
		Style.Colors[ImGuiCol_TabSelectedOverline] = TransparentColor;
		Style.Colors[ImGuiCol_TabDimmed] = TransparentColor;
		Style.Colors[ImGuiCol_TabDimmedSelected] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_TabDimmedSelectedOverline] = TransparentColor;
		Style.Colors[ImGuiCol_DockingPreview] = HeaderColor;
		Style.Colors[ImGuiCol_DockingEmptyBg] = BackroundTransparentColor;
		Style.Colors[ImGuiCol_PlotLines] = TextDimmedColor;
		Style.Colors[ImGuiCol_PlotLinesHovered] = Accent1Color;
		Style.Colors[ImGuiCol_PlotHistogram] = TextDimmedColor;
		Style.Colors[ImGuiCol_PlotHistogramHovered] = Accent1Color;
		Style.Colors[ImGuiCol_TableHeaderBg] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_TableBorderStrong] = TitleColor;
		Style.Colors[ImGuiCol_TableBorderLight] = HeaderColor;
		Style.Colors[ImGuiCol_TableRowBg] = BackroundTransparentColor;
		Style.Colors[ImGuiCol_TableRowBgAlt] = BackroundChildTransparentColor;
		Style.Colors[ImGuiCol_TextLink] = Accent1Color;
		Style.Colors[ImGuiCol_TextSelectedBg] = Accent1Color;
		Style.Colors[ImGuiCol_TreeLines] = TextDimmedColor;
		Style.Colors[ImGuiCol_DragDropTarget] = Accent1Color;
		Style.Colors[ImGuiCol_DragDropTargetBg] = TransparentColor;
		Style.Colors[ImGuiCol_UnsavedMarker] = Accent1Color;
		Style.Colors[ImGuiCol_NavCursor] = Accent1Color;
		Style.Colors[ImGuiCol_NavWindowingHighlight] = Accent1Color;
		Style.Colors[ImGuiCol_NavWindowingDimBg] = BackroundDimmedColor;
		Style.Colors[ImGuiCol_ModalWindowDimBg] = BackroundDimmedColor;
	}

	void SetupForestGreenStyle() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// --- 1. Sizing and Spacing ---
		style.WindowPadding = ImVec2(10.0f, 10.0f);
		style.FramePadding = ImVec2(6.0f, 4.0f);
		style.CellPadding = ImVec2(6.0f, 4.0f);
		style.ItemSpacing = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
		style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
		style.IndentSpacing = 20.0f;
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 12.0f;

		// --- 2. Borders ---
		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;
		style.TabBorderSize = 0.0f;

		// --- 3. Rounding ---
		style.WindowRounding = 6.0f;
		style.ChildRounding = 4.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 4.0f;
		style.ScrollbarRounding = 4.0f;
		style.GrabRounding = 4.0f;
		style.LogSliderDeadzone = 4.0f;
		style.TabRounding = 4.0f;

		// --- 4. Full Color Palette ---

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.85f, 0.90f, 0.85f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.55f, 0.50f, 1.00f);

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.09f, 0.06f, 1.00f); // Deep pine
		colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.11f, 0.08f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.10f, 0.07f, 0.96f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.18f, 0.28f, 0.18f, 0.80f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frames (Inputs, Checkboxes, etc.)
		colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.18f, 0.12f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.30f, 0.18f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.42f, 0.24f, 1.00f);

		// Title Bars
		colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.14f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.26f, 0.14f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.08f, 0.05f, 1.00f);

		// Menus
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.14f, 0.09f, 1.00f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.08f, 0.05f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.38f, 0.25f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.48f, 0.32f, 1.00f);

		// Interactables
		colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.75f, 0.45f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.55f, 0.35f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.70f, 0.45f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.18f, 0.35f, 0.18f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);

		// Separators and Resizing
		colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.18f, 0.35f, 0.18f, 0.80f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.32f, 0.55f, 0.32f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.38f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.15f, 0.08f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);

		// Plots
		colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.85f, 0.50f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.70f, 0.40f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.50f, 0.85f, 0.50f, 1.00f);

		// Tables
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.12f, 0.22f, 0.12f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.20f, 0.35f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.15f, 0.25f, 0.15f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.08f, 0.14f, 0.08f, 0.50f);

		// Misc
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 0.55f, 0.25f, 0.50f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.60f, 0.90f, 0.60f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.80f, 0.40f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.85f, 0.90f, 0.85f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.15f, 0.10f, 0.50f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.08f, 0.05f, 0.60f);

#ifdef IMGUI_HAS_DOCK
		// Docking (If using the docking branch)
		colors[ImGuiCol_DockingPreview] = ImVec4(0.25f, 0.55f, 0.25f, 0.50f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.06f, 0.09f, 0.06f, 1.00f);
#endif
	}

	void SetupImGuiDraculaStyle() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// --- 1. Sizing and Spacing (Clean & Balanced) ---
		style.WindowPadding = ImVec2(10.0f, 10.0f);
		style.FramePadding = ImVec2(6.0f, 4.0f);
		style.ItemSpacing = ImVec2(8.0f, 6.0f);
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 12.0f;

		// --- 2. Borders & Rounding ---
		style.WindowRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 4.0f;
		style.ScrollbarRounding = 12.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;

		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;

		// --- 3. The Dracula Color Palette ---
		// Background: #282a36 | Selection: #44475a | Foreground: #f8f8f2
		// Comment: #6272a4    | Cyan: #8be9fd      | Green: #50fa7b
		// Orange: #ffb86c     | Pink: #ff79c6      | Purple: #bd93f9
		// Red: #ff5555        | Yellow: #f1fa8c

		// Text
		colors[ImGuiCol_Text] = ImVec4(0.97f, 0.97f, 0.95f, 1.00f);         // #f8f8f2
		colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f); // #6272a4

		// Backgrounds
		colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f); // #282a36
		colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.21f, 0.96f);

		// Borders
		colors[ImGuiCol_Border] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f); // #44475a
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frames (Inputs, etc.)
		colors[ImGuiCol_FrameBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);        // #44475a
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f); // #6272a4
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

		// Title Bars
		colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f); // Darker
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);

		// Menus
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);

		// Scrollbars
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

		// Interactables
		colors[ImGuiCol_CheckMark] = ImVec4(0.31f, 0.98f, 0.48f, 1.00f);  // #50fa7b (Green)
		colors[ImGuiCol_SliderGrab] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f); // #bd93f9 (Purple)
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.84f, 0.68f, 1.00f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.47f, 0.78f, 1.00f); // #ff79c6 (Pink)
		colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.37f, 0.62f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.55f, 0.74f, 1.00f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);

		// Tables
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);

		// Misc
		colors[ImGuiCol_PlotLines] = ImVec4(0.55f, 0.91f, 0.99f, 1.00f); // #8be9fd (Cyan)
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);

#ifdef IMGUI_HAS_DOCK
		colors[ImGuiCol_DockingPreview] = ImVec4(0.74f, 0.58f, 0.98f, 0.50f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
#endif
	}
} // namespace Editor::Themes
