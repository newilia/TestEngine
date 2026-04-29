#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/App/EngineContext.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>

#include <algorithm>

namespace Engine {

	void DebugSettingsWidget::Draw() const {
		auto& engine = EngineContext::GetInstance();

		ImGui::SeparatorText("Simulation");
		bool simEnabled = !engine.IsSimPaused();
		if (ImGui::Checkbox("Simulation enabled", &simEnabled)) {
			engine.SetSimPaused(!simEnabled);
		}
		float speedMul = engine.GetSimSpeedMultiplier();
		if (ImGui::SliderFloat("dt multiplier", &speedMul, 0.05f, 8.f, "%.3f")) {
			engine.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
		}

		ImGui::SeparatorText("Timing");
		int fpsLimit = static_cast<int>(engine.GetFramerateLimit());
		if (ImGui::DragInt("Max FPS (window)", &fpsLimit, 1, 0, 10000)) {
			fpsLimit = std::max(0, fpsLimit);
			engine.SetFramerateLimit(static_cast<std::uint32_t>(fpsLimit));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = unlimited)");

		ImGui::Text("Frame dt: %.5f s", static_cast<double>(engine.GetFrameDt().asSeconds()));
		ImGui::Text("Sim dt:   %.5f s", static_cast<double>(engine.GetSimDt().asSeconds()));
		ImGui::Text("Frame dt (raw): %.5f s", static_cast<double>(engine.GetFrameDt(true).asSeconds()));

		const bool hadFixed = engine.GetFixedFrameTime().has_value();
		bool useFixed = hadFixed;
		if (ImGui::Checkbox("Fixed frame dt", &useFixed)) {
			if (useFixed) {
				const float fs = hadFixed ? engine.GetFixedFrameTime()->asSeconds() : (1.f / 60.f);
				engine.SetFixedFrameTime(sf::seconds(std::max(1e-6f, fs)));
			}
			else {
				engine.ResetFixedFrameTime();
			}
		}
		if (engine.GetFixedFrameTime()) {
			float fixedSec = engine.GetFixedFrameTime()->asSeconds();
			if (ImGui::DragFloat("Fixed dt (s)", &fixedSec, 0.00005f, 1e-6f, 0.25f, "%.6f")) {
				engine.SetFixedFrameTime(sf::seconds(std::max(1e-6f, fixedSec)));
			}
		}

		if (const auto ph = engine.GetPhysicsHandler()) {
			ImGui::SeparatorText("Physics");
			bool gravOn = ph->IsGravityEnabled();
			if (ImGui::Checkbox("World gravity", &gravOn)) {
				ph->SetGravityEnabled(gravOn);
			}
			sf::Vector2f g = ph->GetGravity();
			float gxy[2] = {g.x, g.y};
			if (ImGui::DragFloat2("Gravity (px/s^2)", gxy, 10.f, -50000.f, 50000.f, "%.1f")) {
				ph->SetGravity({gxy[0], gxy[1]});
			}
			int subs = ph->GetSubstepCount();
			if (ImGui::SliderInt("Physics substeps / frame", &subs, 1, 16)) {
				ph->SetSubstepCount(std::max(1, subs));
			}
			ImGui::Text("Registered bodies: %d", static_cast<int>(ph->GetAllBodies().size()));
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
