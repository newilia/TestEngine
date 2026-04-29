#pragma once

namespace Engine {

	/// ImGui panel: simulation, debug draw, inverse-square field tuning (`EngineContext` /
	/// `IsotropicInverseSquareField`).
	class DebugSettingsWidget
	{
	public:
		void Draw() const;
	};

} // namespace Engine
