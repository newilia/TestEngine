#pragma once

#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>

class Scene;

namespace Engine {

	struct GpuPointLight
	{
		sf::Vector2f position{};
		sf::Glsl::Vec3 color{};
		float radius = 1.f;
	};

	class SceneLighting
	{
	public:
		static void CollectLights(const Scene& scene);
		static const std::vector<GpuPointLight>& GetLights();

		/// Fills `out` with up to `maxLights` lights affecting `receiverBounds` (world space).
		static void SelectLightsForBounds(const sf::FloatRect& receiverBounds, std::vector<GpuPointLight>& out,
		                                  std::size_t maxLights);

	private:
		static std::vector<GpuPointLight> _lights;
	};

} // namespace Engine
