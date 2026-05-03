#include "TapTool.h"

#include <imgui.h>

bool TapTool::processEvent(const sf::Event& /*event*/) {
	return false;
}

void TapTool::drawToolParametersUi() {
	ImGui::TextUnformatted("Tap forwards input to the scene (DispatchTapAt).");
}
