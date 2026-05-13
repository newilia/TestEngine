#pragma once

class PhysicsProcessor;

namespace Engine {
	class MainContext;

	class DebugSettingsWidget
	{
	public:
		void Draw() const;

	private:
		void DrawSimulationSettings(MainContext& mainContext, PhysicsProcessor* physicsProc) const;
		void DrawPhysicsSettings(MainContext& mainContext, PhysicsProcessor* physicsProc) const;
		void DrawVisualizationSettings(MainContext& mainContext) const;
		void DrawPerformanceInfo(MainContext& mainContext) const;
		void DrawRenderSettings(MainContext& mainContext) const;
	};
} // namespace Engine
