#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Render/SceneLighting.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>

#include <algorithm>

namespace Engine {
	void DebugSettingsWidget::Draw() const {
		auto& mainContext = Engine::MainContext::GetInstance();

		ImGui::SeparatorText("Simulation");
		{
			bool simEnabled = !mainContext.IsSimPaused();
			if (ImGui::Checkbox("Run", &simEnabled)) {
				mainContext.SetSimPaused(!simEnabled);
			}

			if (ImGui::Button("1.0")) {
				mainContext.SetSimSpeedMultiplier(1.0f);
			}
			ImGui::SameLine();
			float speedMul = mainContext.GetSimSpeedMultiplier();
			if (ImGui::SliderFloat("Engine dt multiplier", &speedMul, 0.05f, 4.f, "%.3f")) {
				mainContext.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
			}

			int motionSubsteps = mainContext.GetPhysicsProcessor()->GetMotionSubsteps();
			if (ImGui::SliderInt("Motion substeps", &motionSubsteps, 1, 4)) {
				mainContext.GetPhysicsProcessor()->SetMotionSubsteps(motionSubsteps);
			}
		}

		if (const auto ph = mainContext.GetPhysicsProcessor()) {
			ImGui::SeparatorText("Physics");
			{
				ImGui::Text("Bodies: %d", static_cast<int>(ph->GetAllBodies().size()));

				bool gravOn = ph->IsGravityEnabled();
				if (ImGui::Checkbox("World gravity", &gravOn)) {
					ph->SetGravityEnabled(gravOn);
				}
				sf::Vector2f g = ph->GetGravity();
				float gxy[2] = {g.x, g.y};
				if (ImGui::DragFloat2("Gravity (px/s^2)", gxy, 10.f, -50000.f, 50000.f, "%.1f")) {
					ph->SetGravity({gxy[0], gxy[1]});
				}

				float airFriction = ph->GetAirFriction();
				if (ImGui::DragFloat("Air friction (1/s)", &airFriction, 0.01f, 0.f, 100.f, "%.3f")) {
					ph->SetAirFriction(airFriction);
				}

				if (auto field = ph->GetAttractionField()) {
					float strength = field->GetGlobalStrengthScale();
					if (ImGui::DragFloat("Attraction field strength", &strength, 2.f, -10000.f, 10000, "%.2f")) {
						field->SetGlobalStrengthScale(strength);
					}
					bool massCoupling = field->GetUseMassCoupling();
					if (ImGui::Checkbox("Attraction: mass couples sources", &massCoupling)) {
						field->SetUseMassCoupling(massCoupling);
					}
					/*float softEps = field->GetSofteningEps();
				if (ImGui::DragFloat("Field softening eps", &softEps, 0.25f, 0.1f, 1.0e3f, "%.2f")) {
					field->SetSofteningEps(softEps);
				}*/
				}
			}
		}

		ImGui::SeparatorText("Performance");
		{
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
		}

		ImGui::SeparatorText("Render");
		{
			auto& sceneLighting = SceneLighting::GetInstance();
			bool lightingEnabled = sceneLighting.IsEnabled();
			if (ImGui::Checkbox("Lighting enabled", &lightingEnabled)) {
				sceneLighting.SetEnabled(lightingEnabled);
			}

			if (ImGui::Button("1.0##LightDistanceScale")) {
				sceneLighting.SetDistanceRangeScale(1.0f);
			}
			ImGui::SameLine();
			float distanceScale = sceneLighting.GetDistanceRangeScale();
			if (ImGui::SliderFloat("Light distance scale", &distanceScale, 0.1f, 3.f, "%.2f")) {
				sceneLighting.SetDistanceRangeScale(distanceScale);
			}

			if (ImGui::Button("1.0##LightIntensityScale")) {
				sceneLighting.SetGlobalIntensityScale(1.0f);
			}
			ImGui::SameLine();
			float intensityScale = sceneLighting.GetGlobalIntensityScale();
			if (ImGui::SliderFloat("Light intensity scale", &intensityScale, 0.1f, 3.f, "%.2f")) {
				sceneLighting.SetGlobalIntensityScale(intensityScale);
			}

			constexpr const char* blendModeLabels[] = {"Additive", "Screen"};
			int blendMode = static_cast<int>(sceneLighting.GetBlendMode());
			if (ImGui::Combo("Lighting blend mode", &blendMode, blendModeLabels, IM_ARRAYSIZE(blendModeLabels))) {
				blendMode = std::clamp(blendMode, 0, static_cast<int>(IM_ARRAYSIZE(blendModeLabels)) - 1);
				sceneLighting.SetBlendMode(static_cast<LightingBlendMode>(blendMode));
			}
		}
	}

} // namespace Engine
