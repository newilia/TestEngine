#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>

#include <algorithm>

namespace Engine {
	void DebugSettingsWidget::Draw() const {
		auto& mainContext = Engine::MainContext::GetInstance();

		ImGui::SeparatorText("Simulation");
		bool simEnabled = !mainContext.IsSimPaused();
		if (ImGui::Checkbox("Simulation enabled", &simEnabled)) {
			mainContext.SetSimPaused(!simEnabled);
		}
		float speedMul = mainContext.GetSimSpeedMultiplier();
		if (ImGui::SliderFloat("dt multiplier", &speedMul, 0.05f, 8.f, "%.3f")) {
			mainContext.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
		}

		ImGui::SeparatorText("Timing");
		bool vsync = mainContext.IsVerticalSyncEnabled();
		if (ImGui::Checkbox("VSync", &vsync)) {
			mainContext.SetVerticalSyncEnabled(vsync);
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(sync to display; pair with Max FPS 0)");

		bool isFpsLimitEnabled = mainContext.IsFramerateLimitEnabled();
		if (ImGui::Checkbox("FPS limit enabled", &isFpsLimitEnabled)) {
			mainContext.SetFramerateLimitEnabled(isFpsLimitEnabled);
		}

		int fpsLimit = static_cast<int>(mainContext.GetFramerateLimit());
		if (ImGui::SliderInt("FPS limit", &fpsLimit, 30, 200)) {
			mainContext.SetFramerateLimit(static_cast<std::uint32_t>(fpsLimit));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = off; meaningful when VSync is off)");

		int tickHz = static_cast<int>(mainContext.GetTargetTickRate());
		if (ImGui::SliderInt("Logic tick rate (Hz)", &tickHz, 30, 500)) {
			tickHz = std::max(0, tickHz);
			mainContext.SetTargetTickRate(static_cast<std::uint32_t>(tickHz));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = unlimited: one variable step per frame)");

		ImGui::Text("Frame dt: %.3f s (%.1f fps)", static_cast<double>(mainContext.GetFrameDt().asSeconds()),
		            mainContext.GetCurrentFps());

		ImGui::Text("Tick dt:  %.3f s (%.1f fps)", static_cast<double>(mainContext.GetSimTickDt().asSeconds()),
		            mainContext.GetCurrentTickRate());

		if (const auto ph = mainContext.GetPhysicsProcessor()) {
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
		bool debugDraw = mainContext.IsDebugDrawEnabled();
		if (ImGui::Checkbox("Debug draw (velocities, labels, ...)", &debugDraw)) {
			mainContext.SetDebugDrawEnabled(debugDraw);
		}
		float forceArrowScale = mainContext.GetFieldForceDebugArrowScale();
		if (ImGui::SliderFloat("Field force arrow scale", &forceArrowScale, 1e-5f, 20.f, "%.5f",
		                       ImGuiSliderFlags_Logarithmic)) {
			mainContext.SetFieldForceDebugArrowScale(forceArrowScale);
		}
		if (const auto ph = mainContext.GetPhysicsProcessor()) {
			if (auto field = ph->GetAttractionField()) {
				float strength = field->GetGlobalStrengthScale();
				if (ImGui::DragFloat("Attraction field strength", &strength, 2.f, -10000.f, 10000, "%.2f")) {
					field->SetGlobalStrengthScale(strength);
				}
				bool massCoupling = field->GetUseMassCoupling();
				if (ImGui::Checkbox("Field: mass couples sources", &massCoupling)) {
					field->SetUseMassCoupling(massCoupling);
				}
				float softEps = field->GetSofteningEps();
				if (ImGui::DragFloat("Field softening eps", &softEps, 0.25f, 0.1f, 1.0e3f, "%.2f")) {
					field->SetSofteningEps(softEps);
				}
			}
		}
	}

} // namespace Engine
