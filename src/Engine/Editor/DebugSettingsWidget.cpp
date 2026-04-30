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
		bool vsync = engine.IsVerticalSyncEnabled();
		if (ImGui::Checkbox("VSync", &vsync)) {
			engine.SetVerticalSyncEnabled(vsync);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(sync to display; pair with Max FPS 0)");

		bool isFpsLimitEnabled = engine.IsFramerateLimitEnabled();
		if (ImGui::Checkbox("FPS limit enabled", &isFpsLimitEnabled)) {
			engine.SetFramerateLimitEnabled(isFpsLimitEnabled);
		}

		int fpsLimit = static_cast<int>(engine.GetFramerateLimit());
		if (ImGui::DragInt("FPS limit", &fpsLimit, 1, 30, 200)) {
			engine.SetFramerateLimit(static_cast<std::uint32_t>(fpsLimit));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = off; meaningful when VSync is off)");

		int tickHz = static_cast<int>(engine.GetTargetTickRateHz());
		if (ImGui::DragInt("Logic tick rate (Hz)", &tickHz, 10, 0, 10000)) {
			tickHz = std::max(0, tickHz);
			engine.SetTargetTickRateHz(static_cast<std::uint32_t>(tickHz));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = unlimited: one variable step per frame)");

		ImGui::Text("Frame dt: %.3f s (%.1f fps)", static_cast<double>(engine.GetFrameDt().asSeconds()),
		            engine.GetFps());

		ImGui::Text("Tick dt:  %.3f s (%.1f fps)", static_cast<double>(engine.GetSimTickDt().asSeconds()),
		            engine.GetTickRate());

		if (const auto ph = engine.GetPhysicsProcessor()) {
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
			ImGui::Text("Registered bodies: %d", static_cast<int>(ph->GetAllBodies().size()));
		}

		ImGui::SeparatorText("Debug & field");
		bool debugDraw = engine.IsDebugDrawEnabled();
		if (ImGui::Checkbox("Debug draw (velocities, labels, ...)", &debugDraw)) {
			engine.SetDebugEnabled(debugDraw);
		}
		float forceArrowScale = engine.GetFieldForceDebugArrowScale();
		if (ImGui::SliderFloat("Field force arrow scale", &forceArrowScale, 1e-5f, 20.f, "%.5f",
		                       ImGuiSliderFlags_Logarithmic)) {
			engine.SetFieldForceDebugArrowScale(forceArrowScale);
		}
		if (const auto ph = engine.GetPhysicsProcessor()) {
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
