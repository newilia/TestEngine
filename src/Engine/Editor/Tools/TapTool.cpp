#include "TapTool.h"

#include <imgui.h>

bool TapTool::processEvent(const sf::Event& /*event*/) {
	return false;
}

void TapTool::drawToolParametersUi() {
	ImGui::TextUnformatted("Pass tap event to scene");
}
