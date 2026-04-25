#include "Engine/Editor/NodeInspectorWidget.h"

#include <imgui.h>

namespace {

	void TextLabelValue(const char* label, const char* value) {
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		ImGui::TextUnformatted(value);
	}

} // namespace

namespace Engine {

	void NodeInspectorWidget::Draw(const std::shared_ptr<SceneNode>& node) const {
		if (!node) {
			ImGui::TextUnformatted("No node selected");
			return;
		}
		const char* const nameCStr = node->GetName().empty() ? "<unnamed>" : node->GetName().c_str();
		ImGui::SeparatorText("Node");
		ImGui::Text("Name: ");
		ImGui::SameLine();
		ImGui::TextUnformatted(nameCStr);
		ImGui::Text("Children: %zu", node->GetChildren().size());
		if (const auto parent = node->GetParent()) {
			const char* pName = parent->GetName().empty() ? "<unnamed>" : parent->GetName().c_str();
			ImGui::Text("Parent: ");
			ImGui::SameLine();
			ImGui::TextUnformatted(pName);
		}
		else {
			ImGui::TextUnformatted("Parent: (root)");
		}

		ImGui::SeparatorText("Transform");
		const sf::Vector2f local = node->getPosition();
		const sf::Vector2f world = node->GetPosGlobal();
		const sf::Vector2f scale = node->getScale();
		ImGui::Text("Local position:  (%.2f, %.2f)", static_cast<double>(local.x), static_cast<double>(local.y));
		ImGui::Text("Global position: (%.2f, %.2f)", static_cast<double>(world.x), static_cast<double>(world.y));
		ImGui::Text("Scale: (%.3f, %.3f)", static_cast<double>(scale.x), static_cast<double>(scale.y));
		const double rotDeg = static_cast<double>(node->getRotation().asDegrees());
		ImGui::Text("Rotation: %.2f deg", rotDeg);

		ImGui::SeparatorText("Attachments");
		const bool hasVisual = node->GetVisual() != nullptr;
		const bool hasCollider = node->FindShapeCollider() != nullptr;
		TextLabelValue("Visual: ", hasVisual ? "yes" : "no");
		TextLabelValue("Shape collider: ", hasCollider ? "yes" : "no");
	}

} // namespace Engine
