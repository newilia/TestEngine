#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/App/EngineInterface.h"

#include <imgui.h>

#include <algorithm>

namespace Engine {

	void DebugSettingsWidget::Draw() const {
		auto& engine = EngineContext::Instance();
		ImGui::SeparatorText("Simulation");
		bool simEnabled = !engine.IsSimPaused();
		if (ImGui::Checkbox("Simulation enabled", &simEnabled)) {
			engine.SetSimPaused(!simEnabled);
		}
		float speedMul = engine.GetSimSpeedMultiplier();
		if (ImGui::SliderFloat("dt multiplier", &speedMul, 0.05f, 8.f, "%.3f")) {
			engine.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
		}
		ImGui::SeparatorText("Debug & field");
		bool debugDraw = engine.IsDebugEnabled();
		if (ImGui::Checkbox("Debug draw (velocities, labels, ...)", &debugDraw)) {
			engine.SetDebugEnabled(debugDraw);
		}
		float forceArrowScale = engine.GetFieldForceDebugArrowScale();
		if (ImGui::SliderFloat("Field force arrow scale", &forceArrowScale, 1e-5f, 20.f, "%.5f",
		                       ImGuiSliderFlags_Logarithmic)) {
			engine.SetFieldForceDebugArrowScale(forceArrowScale);
		}
		if (const auto ph = engine.GetPhysicsHandler()) {
			if (auto field = ph->GetIsotropicInverseSquareField()) {
				float strength = field->GetGlobalStrengthScale();
				if (ImGui::DragFloat("Inverse-square field strength", &strength, 2.f, 0.f, 1.0e6f, "%.2f")) {
					field->SetGlobalStrengthScale(strength);
				}
				bool massCoupling = field->GetUseMassCoupling();
				if (ImGui::Checkbox("Field: mass couples sources", &massCoupling)) {
					field->SetUseMassCoupling(massCoupling);
				}
				float softEps = field->GetSofteningEps();
				if (ImGui::DragFloat("Field softening ε", &softEps, 0.25f, 0.1f, 1.0e3f, "%.2f")) {
					field->SetSofteningEps(softEps);
				}
			}
		}
	}

} // namespace Engine
