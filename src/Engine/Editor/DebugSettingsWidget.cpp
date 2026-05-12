#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Render/SceneLighting.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>

namespace Engine {
	void DebugSettingsWidget::Draw() const {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto physicsProc = mainContext.GetPhysicsProcessor();

		ImGui::SeparatorText("Simulation");
		{
			bool simEnabled = !mainContext.IsSimPaused();
			if (ImGui::Checkbox("Run", &simEnabled)) {
				mainContext.SetSimPaused(!simEnabled);
			}
			if (!simEnabled) {
				ImGui::SameLine();
				static float dt = 0.05;
				ImGui::SliderFloat("dt", &dt, 0.001, 0.1);
				ImGui::SameLine();
				if (ImGui::Button("Step")) {
					int substeps = physicsProc->GetSimulationSubsteps();
					physicsProc->SetSimulationSubsteps(1);
					physicsProc->Update(sf::seconds(dt));
					physicsProc->SetSimulationSubsteps(substeps);
				}
			}

			if (ImGui::Button("1.0")) {
				mainContext.SetSimSpeedMultiplier(1.0f);
			}
			ImGui::SameLine();
			float speedMul = mainContext.GetSimSpeedMultiplier();
			if (ImGui::SliderFloat("Engine dt multiplier", &speedMul, 0.05f, 4.f, "%.3f")) {
				mainContext.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
			}

			int substeps = physicsProc->GetSimulationSubsteps();
			if (ImGui::SliderInt("Motion substeps", &substeps, 1, 4)) {
				physicsProc->SetSimulationSubsteps(substeps);
			}
		}

		if (const auto ph = physicsProc) {
			ImGui::SeparatorText("Physics");
			{
				ImGui::Text("Bodies: %d", static_cast<int>(ph->GetAllBodies().size()));

				bool gravOn = ph->IsGravityEnabled();
				if (ImGui::Checkbox("##World gravity", &gravOn)) {
					ph->SetGravityEnabled(gravOn);
				}
				ImGui::SameLine();

				{
					auto& g = *ImGui::GetCurrentContext();
					auto width = g.CurrentWindow->DC.ItemWidthDefault;
					width -= g.FontSize + g.Style.FramePadding.y * 2 + g.Style.ItemSpacing.x;
					ImGui::SetNextItemWidth(width);
				}
				sf::Vector2f g = ph->GetGravity();
				float gxy[2] = {g.x, g.y};
				if (ImGui::DragFloat2("Gravity (px/s^2)", gxy, 10.f, -50000.f, 50000.f, "%.1f")) {
					ph->SetGravity({gxy[0], gxy[1]});
				}

				float airFriction = ph->GetAirFriction();
				if (ImGui::SliderFloat("Air friction (1/s)", &airFriction, 0.f, 0.1f, "%.4f")) {
					ph->SetAirFriction(airFriction);
				}

				if (auto field = ph->GetAttractionField()) {
					ImGui::Text("Attraction:");
					bool massCoupling = field->GetUseMassCoupling();
					float strength = field->GetGlobalStrengthScale();
					if (ImGui::DragFloat("Field strength", &strength, 2.f, -10000.f, 10000, "%.2f")) {
						field->SetGlobalStrengthScale(strength);
					}

					float softEps = field->GetSofteningEps();
					if (ImGui::DragFloat("Field softening eps", &softEps, 0.25f, 0.1f, 1.0e3f, "%.2f")) {
						field->SetSofteningEps(softEps);
					}
					if (ImGui::Checkbox("Mass couples sources", &massCoupling)) {
						field->SetUseMassCoupling(massCoupling);
					}
				}
			}
		}

		ImGui::SeparatorText("Performance");
		{
			bool vsync = mainContext.IsVerticalSyncEnabled();
			if (ImGui::Checkbox("VSync", &vsync)) {
				mainContext.SetVerticalSyncEnabled(vsync);
			}
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
