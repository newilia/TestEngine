#include "Engine/Editor/DebugSettingsWidget.h"

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/ImGuiUtils.h"
#include "Engine/Render/SceneLighting.h"
#include "Engine/Simulation/PhysicsProcessor.h"

#include <SFML/System/Time.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstdint>

namespace Engine {
	void DebugSettingsWidget::Draw() const {
		auto& mainContext = Engine::MainContext::GetInstance();
		auto physicsProc = mainContext.GetPhysicsProcessor().get();
		DrawSimulationSettings(mainContext, physicsProc);
		DrawPhysicsSettings(mainContext, physicsProc);
		DrawVisualizationSettings(mainContext);
		DrawPerformanceInfo(mainContext);
		DrawRenderSettings(mainContext);
	}

	void DebugSettingsWidget::DrawSimulationSettings(MainContext& mainContext, PhysicsProcessor* physicsProc) const {
		ImGui::SeparatorText("Simulation");
		{
			bool simEnabled = !mainContext.IsSimPaused();
			if (ImGui::Checkbox("Run", &simEnabled)) {
				mainContext.SetSimPaused(!simEnabled);
			}
			if (!simEnabled) {
				ImGui::SameLine();
				static float dt = 0.01;
				ImGui::SliderFloat("dt", &dt, 0.001, 0.0167);
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
			ImGui::SetNextItemWidth(ImGuiUtils::GetRemainingWidth());

			float speedMul = mainContext.GetSimSpeedMultiplier();
			if (ImGui::DragFloat(
			        "Sim dt multiplier", &speedMul, 0.01f, 0.01f, 4.f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
				mainContext.SetSimSpeedMultiplier(std::max(1e-4f, speedMul));
			}

			int substeps = physicsProc->GetSimulationSubsteps();
			if (ImGui::SliderInt("Motion substeps", &substeps, 1, 4)) {
				physicsProc->SetSimulationSubsteps(substeps);
			}
		}
	}

	void DebugSettingsWidget::DrawPhysicsSettings(MainContext& mainContext, PhysicsProcessor* physicsProc) const {
		ImGui::SeparatorText("Physics");
		{
			ImGui::Text("Bodies: %d", static_cast<int>(physicsProc->GetAllBodies().size()));

			{
				float systemImpulse = 0.f;
				float systemEnergy = 0.f;
				for (const auto& wBody : physicsProc->GetAllBodies()) {
					if (auto body = wBody.lock()) {
						auto vel = body->GetVelocity().length();
						systemImpulse += body->GetMass() * vel;
						systemEnergy += body->GetMass() * vel * vel / 2;
					}
				}
				ImGui::Text("P = %.0f, E = %.0f", systemImpulse, systemEnergy);
			}

			bool gravOn = physicsProc->IsGravityEnabled();
			if (ImGui::Checkbox("##World gravity", &gravOn)) {
				physicsProc->SetGravityEnabled(gravOn);
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGuiUtils::GetRemainingWidth());

			sf::Vector2f g = physicsProc->GetGravity();
			float gxy[2] = {g.x, g.y};
			if (ImGui::DragFloat2("Gravity (px/s^2)", gxy, 10.f, -50000.f, 50000.f, "%.1f")) {
				physicsProc->SetGravity({gxy[0], gxy[1]});
			}

			float airFriction = physicsProc->GetAirFriction();
			if (ImGui::SliderFloat("Air friction (1/s)", &airFriction, 0.f, 0.1f, "%.4f")) {
				physicsProc->SetAirFriction(airFriction);
			}

			ImGui::SeparatorText("Collision tuning (temp)");
			ImGui::SliderFloat("Displacement factor", &gCollisionTuning.displacementFactor, 0.f, 1.f);
			ImGui::DragFloat("Rigid sep. rate", &gCollisionTuning.rigidSeparationRate, 10.f, 1.f, 10000.f);
			ImGui::DragFloat("Soft sep. rate", &gCollisionTuning.softSeparationRate, 0.1f, 0.01f, 100.f);
			ImGui::DragFloat("Resting speed threshold", &gCollisionTuning.restingSpeedThreshold, 0.1f, 0.f, 50.f);

			if (auto field = physicsProc->GetAttractionField()) {
				ImGui::Text("Attraction:");
				bool massCoupling = field->GetUseMassCoupling();
				float strength = field->GetGlobalStrengthScale();
				if (ImGui::DragFloat("Field strength", &strength, 0.2f, -100, 100, "%.2f")) {
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

	void DebugSettingsWidget::DrawVisualizationSettings(MainContext& mainContext) const {
		ImGui::SeparatorText("Visualization");
		{
			auto& viz = Editor::GetInstance().GetPhysicsVisualizer();
			bool showVel = viz.IsVelocityVisible();
			if (ImGui::Checkbox("Show velocity", &showVel)) {
				viz.SetVelocityVisible(showVel);
			}
			if (showVel) {
				float velScale = viz.GetVelocityScale();
				if (ImGui::SliderFloat("Velocity scale", &velScale, 1e-3f, 2.f, "%.3f")) {
					viz.SetVelocityScale(velScale);
				}
			}

			bool showAccel = viz.IsForceVisible();
			if (ImGui::Checkbox("Show force", &showAccel)) {
				viz.SetForceVisible(showAccel);
			}
			if (showAccel) {
				float accelScale = viz.GetForceScale();
				if (ImGui::SliderFloat("Force scale", &accelScale, 1e-3f, 2.f, "%.3f")) {
					viz.SetForceScale(accelScale);
				}
			}

			bool showField = viz.IsAttractionFieldVisible();
			if (ImGui::Checkbox("Show attraction field", &showField)) {
				viz.SetAttractionFieldVisible(showField);
			}
			if (showField) {
				int gridMode = static_cast<int>(viz.GetAttractionFieldGridLayout());
				const char* gridLabels[] = {"Square", "Triangular"};
				if (ImGui::Combo("Field grid", &gridMode, gridLabels, IM_ARRAYSIZE(gridLabels))) {
					gridMode = std::clamp(gridMode, 0, IM_ARRAYSIZE(gridLabels) - 1);
					viz.SetAttractionFieldGridLayout(static_cast<FieldGridLayout>(gridMode));
				}
				float fSpacing = viz.GetAttractionFieldSpacingPx();
				if (ImGui::SliderFloat("Field spacing (px)", &fSpacing, 8.f, 256.f, "%.0f")) {
					viz.SetAttractionFieldSpacingPx(fSpacing);
				}
				float fArrow = viz.GetAttractionFieldArrowScale();
				if (ImGui::SliderFloat("Field arrow scale", &fArrow, 1e-3f, 10.f, "%.3f")) {
					viz.SetAttractionFieldArrowScale(fArrow);
				}
				float softCap = viz.GetAttractionFieldArrowLengthSoftCap();
				if (ImGui::DragFloat(
				        "Field arrow soft cap (world)", &softCap, softCap * 0.05f + 2.f, 0.f, 1.0e5f, "%.1f")) {
					viz.SetAttractionFieldArrowLengthSoftCap(std::max(0.f, softCap));
				}
				float compress = viz.GetAttractionFieldArrowLengthCompress();
				if (ImGui::SliderFloat("Field arrow compress", &compress, 0.f, 1.f, "%.2f")) {
					viz.SetAttractionFieldArrowLengthCompress(compress);
				}
				float paletteSpan = viz.GetAttractionFieldPaletteSpan();
				if (ImGui::DragFloat(
				        "Field palette span", &paletteSpan, paletteSpan * 0.02f + 50.f, 0.f, 5.0e6f, "%.0f")) {
					viz.SetAttractionFieldPaletteSpan(std::max(0.f, paletteSpan));
				}

				const auto clamp255 = [](float x) {
					return static_cast<std::uint8_t>(std::clamp(std::lround(x * 255.f), 0L, 255L));
				};
				auto colorEdit = [&](const char* label, sf::Color c, void (*apply)(PhysicsVisualizer&, sf::Color)) {
					float rgba[4] = {c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f};
					if (ImGui::ColorEdit4(label, rgba, ImGuiColorEditFlags_Float)) {
						apply(
						    viz, sf::Color{clamp255(rgba[0]), clamp255(rgba[1]), clamp255(rgba[2]), clamp255(rgba[3])});
					}
				};
				colorEdit(
				    "Field color (weak)", viz.GetAttractionFieldPaletteWeak(), [](PhysicsVisualizer& v, sf::Color col) {
					    v.SetAttractionFieldPaletteWeak(col);
				    });
				colorEdit(
				    "Field color (mid)", viz.GetAttractionFieldPaletteMid(), [](PhysicsVisualizer& v, sf::Color col) {
					    v.SetAttractionFieldPaletteMid(col);
				    });
				colorEdit("Field color (strong)", viz.GetAttractionFieldPaletteStrong(),
				    [](PhysicsVisualizer& v, sf::Color col) {
					    v.SetAttractionFieldPaletteStrong(col);
				    });

				ImGui::TextUnformatted("Field probe groups");
				auto groups = viz.GetAttractionFieldProbeGroups();
				for (int i = 0; i < PhysicsBodyBehaviour::kGroupsCount; ++i) {
					ImGui::PushID(i);
					bool on = groups.test(static_cast<std::size_t>(i));
					if (i > 0) {
						ImGui::SameLine();
					}
					char bitLabel[2];
					bitLabel[0] = static_cast<char>('0' + i);
					bitLabel[1] = '\0';
					if (ImGui::Checkbox(bitLabel, &on)) {
						groups.set(static_cast<std::size_t>(i), on);
						viz.SetAttractionFieldProbeGroups(groups);
					}
					ImGui::PopID();
				}
			}
		}
	}

	void DebugSettingsWidget::DrawPerformanceInfo(MainContext& mainContext) const {
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
	}

	void DebugSettingsWidget::DrawRenderSettings(MainContext& mainContext) const {
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
			ImGui::SetNextItemWidth(ImGuiUtils::GetRemainingWidth());
			float distanceScale = sceneLighting.GetDistanceRangeScale();
			if (ImGui::SliderFloat("Light distance scale", &distanceScale, 0.1f, 3.f, "%.2f")) {
				sceneLighting.SetDistanceRangeScale(distanceScale);
			}

			if (ImGui::Button("1.0##LightIntensityScale")) {
				sceneLighting.SetGlobalIntensityScale(1.0f);
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGuiUtils::GetRemainingWidth());
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
