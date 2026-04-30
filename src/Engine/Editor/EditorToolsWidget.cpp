#include "EditorToolsWidget.h"

#include "Engine/Editor/EditorToolManager.h"
#include "Engine/Editor/Tools/PullTool.h"

#include <imgui.h>

namespace Engine {

	void EditorToolsWidget::Draw(EditorToolManager& tools) {
		ImGui::TextUnformatted("Tools");
		ImGui::Separator();

		const char* names[] = {"Tap", "Select", "Pull", "Move"};
		int active = tools.GetActiveToolIndex();
		for (int i = 0; i < EditorToolManager::kToolCount; ++i) {
			const std::string label = EditorToolManager::FormatToolPaletteLabel(i, names[i]);
			if (ImGui::RadioButton(label.c_str(), active == i)) {
				tools.SetActiveToolIndex(i);
				active = i;
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("Parameters");

		switch (active) {
		case 2: {
			auto* pull = tools.GetPullTool();
			if (pull) {
				int mode = pull->GetPullModeIndex();
				ImGui::TextUnformatted("Pull mode");
				if (ImGui::RadioButton("Position snap", mode == 0)) {
					mode = 0;
				}
				if (ImGui::RadioButton("Force", mode == 1)) {
					mode = 1;
				}
				if (ImGui::RadioButton("Set velocity", mode == 2)) {
					mode = 2;
				}
				pull->SetPullModeIndex(mode);

				float scale = pull->GetPullForceScale();
				if (ImGui::SliderFloat("Pull force scale", &scale, 0.01f, 100.f, "%.3f")) {
					pull->SetPullForceScale(scale);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Multiplier for pull force (Force mode); base strength is 100000.");
				}

				bool dbg = pull->IsDebugArrowEnabled();
				if (ImGui::Checkbox("Debug draw arrow", &dbg)) {
					pull->SetDebugArrowEnabled(dbg);
				}
			}
			break;
		}
		case 0:
			ImGui::TextUnformatted("Tap forwards input to the scene (DispatchTapAt).");
			break;
		case 1:
			ImGui::TextUnformatted("Click a body or tap target to select it in the hierarchy.");
			break;
		case 3:
			ImGui::TextUnformatted("Drag bodies with LMB; velocity is cleared while moving.");
			break;
		default:
			break;
		}
	}

} // namespace Engine
