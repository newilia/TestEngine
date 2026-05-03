#include "EditorToolsWidget.h"

#include "Engine/Editor/EditorToolManager.h"

#include <imgui.h>

namespace Engine {

	void EditorToolsWidget::Draw(EditorToolManager& toolManager) {
		ImGui::TextUnformatted("Tools");
		ImGui::Separator();

		const char* names[] = {"Tap", "Select", "Pull", "Move", "Polygon"};
		for (int i = 0; i < EditorToolManager::kToolCount; ++i) {
			const std::string label = EditorToolManager::FormatToolPaletteLabel(i, names[i]);
			if (ImGui::RadioButton(label.c_str(), toolManager.GetActiveToolIndex() == i)) {
				toolManager.SetActiveToolIndex(i);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("Parameters");

		toolManager.DrawActiveToolParametersUi();
	}

} // namespace Engine
