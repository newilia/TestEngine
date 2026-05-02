#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/App/MainContext.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>

#include <algorithm>

namespace Engine {
	void DebugSettingsWidget::Draw() const {
		auto& engine = Engine::MainContext::GetInstance();

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
		if (ImGui::SliderInt("FPS limit", &fpsLimit, 30, 200)) {
			engine.SetFramerateLimit(static_cast<std::uint32_t>(fpsLimit));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = off; meaningful when VSync is off)");

		int tickHz = static_cast<int>(engine.GetTargetTickRate());
		if (ImGui::SliderInt("Logic tick rate (Hz)", &tickHz, 30, 500)) {
			tickHz = std::max(0, tickHz);
			engine.SetTargetTickRate(static_cast<std::uint32_t>(tickHz));
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(0 = unlimited: one variable step per frame)");

		ImGui::Text("Frame dt: %.3f s (%.1f fps)", static_cast<double>(engine.GetFrameDt().asSeconds()),
		            engine.GetCurrentFps());

		ImGui::Text("Tick dt:  %.3f s (%.1f fps)", static_cast<double>(engine.GetSimTickDt().asSeconds()),
		            engine.GetCurrentTickRate());

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
			engine.SetDebugDrawEnabled(debugDraw);
		}
		float forceArrowScale = engine.GetFieldForceDebugArrowScale();
		if (ImGui::SliderFloat("Field force arrow scale", &forceArrowScale, 1e-5f, 20.f, "%.5f",
		                       ImGuiSliderFlags_Logarithmic)) {
			engine.SetFieldForceDebugArrowScale(forceArrowScale);
		}
		if (const auto ph = engine.GetPhysicsProcessor()) {
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
