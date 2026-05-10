#pragma once

#include "Engine/Core/Singleton.h"

#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <list>
#include <memory>
#include <utility>
#include <vector>

class PointLightBehaviour;

namespace Engine {

	enum class LightingBlendMode : std::uint8_t
	{
		Additive = 0,
		Screen = 1
	};

	struct GpuPointLight
	{
		sf::Vector2f position{};
		sf::Glsl::Vec3 color{};
		float radius = 1.f;
	};

	class SceneLighting : public Singleton<SceneLighting>
	{
		friend class Singleton<SceneLighting>;

	public:
		bool IsEnabled() const;
		void SetEnabled(bool enabled);
		float GetDistanceRangeScale() const;
		void SetDistanceRangeScale(float scale);
		float GetGlobalIntensityScale() const;
		void SetGlobalIntensityScale(float scale);
		LightingBlendMode GetBlendMode() const;
		void SetBlendMode(LightingBlendMode mode);

		void RegisterPointLight(std::shared_ptr<PointLightBehaviour> light);
		void UnregisterPointLight(PointLightBehaviour* light);

		void PrepareLights();
		const std::vector<GpuPointLight>& GetLights() const;

		/// Fills `out` with up to `maxLights` lights affecting `receiverBounds` (world space).
		void SelectLightsForBounds(
		    const sf::FloatRect& receiverBounds, std::vector<GpuPointLight>& out, std::size_t maxLights) const;

	private:
		SceneLighting() = default;

		bool _enabled = true;
		float _distanceRangeScale = 1.f;
		float _globalIntensityScale = 1.f;
		LightingBlendMode _blendMode = LightingBlendMode::Screen;
		std::list<std::weak_ptr<PointLightBehaviour>> _lightSources;
		std::vector<GpuPointLight> _lights;

		mutable std::vector<std::pair<float, std::size_t>> _selectScratch;
	};

} // namespace Engine
