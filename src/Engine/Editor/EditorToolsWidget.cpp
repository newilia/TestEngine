#include "EditorToolsWidget.h"

#include "Engine/Editor/EditorToolManager.h"
#include "Engine/Editor/Tools/PullTool.h"

#include <imgui.h>

namespace Engine {

	void EditorToolsWidget::Draw(EditorToolManager& toolManager) {
		ImGui::TextUnformatted("Tools");
		ImGui::Separator();

		const char* names[] = {"Tap", "Select", "Pull", "Move", "Polygon"};
		int active = toolManager.GetActiveToolIndex();
		for (int i = 0; i < EditorToolManager::kToolCount; ++i) {
			const std::string label = EditorToolManager::FormatToolPaletteLabel(i, names[i]);
			if (ImGui::RadioButton(label.c_str(), active == i)) {
				toolManager.SetActiveToolIndex(i);
				active = i;
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("Parameters");

		switch (active) {
		case 2: {
			/* TODO move tool inpector to the tool itself */
			auto* pull = toolManager.GetPullTool();
			if (pull) {
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
			ImGui::TextUnformatted("Drag physical bodies with LMB; velocity is cleared while moving.");
			break;
		case 4:
			ImGui::TextUnformatted("Draw a convex shape");
			break;
		default:
			break;
		}
	}

} // namespace Engine
