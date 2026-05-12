#include "TapTool.h"

#include <imgui.h>

bool TapTool::ProcessEvent(const sf::Event& /*event*/) {
	return false;
}

void TapTool::DrawToolParametersUi() {
	ImGui::TextUnformatted("Pass tap event to scene");
}
