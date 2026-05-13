#include "EditorToolsWidget.h"

#include "Engine/Editor/EditorToolManager.h"

#include <imgui.h>

namespace Engine {

	void EditorToolsWidget::Draw(EditorToolManager& toolManager) {
		for (int i = 0; i < toolManager.GetToolCount(); ++i) {
			const std::string label = toolManager.GetToolFormattedName(i);
			bool isActive = toolManager.GetActiveToolIndex() == i;
			if (isActive) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
			}
			if (ImGui::Button(label.c_str())) {
				toolManager.SetActiveToolIndex(i);
			}
			ImGui::SameLine();
			if (isActive) {
				ImGui::PopStyleColor();
			}
		}
		ImGui::NewLine();
		ImGui::SeparatorText("Parameters");
		toolManager.DrawActiveToolParametersUi();
	}

} // namespace Engine
