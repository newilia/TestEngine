#pragma once

#include "Engine/Core/Singleton.h"
#include "Engine/Render/ViewportFullscreenEffect.h"

#include <SFML/Graphics/Shader.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

class RadialUvWarpBehaviour;

namespace Engine {

	class RadialUvWarpFullscreenPass final : public IViewportFullscreenEffect,
	                                         public Singleton<RadialUvWarpFullscreenPass>
	{
		friend class Singleton<RadialUvWarpFullscreenPass>;

	public:
		static constexpr std::size_t kMaxWarpCenters = 128u;

		bool IsShaderReady() const;
		bool ShouldUseEffect(const Scene& scene) const;
		void Prepare(const std::shared_ptr<Scene>& scene, const ViewportFullscreenPresentContext& ctx) override;
		void Apply(const sf::Texture& inputTexture, sf::RenderTarget& outputTarget,
		    const ViewportFullscreenPresentContext& ctx) override;

	private:
		RadialUvWarpFullscreenPass();
		bool TryLoadShader();

	private:
		bool _shaderReady = false;
		sf::Shader _shader{};
		std::vector<std::shared_ptr<RadialUvWarpBehaviour>> _activeWarps{};
		std::array<sf::Glsl::Vec2, kMaxWarpCenters> _warpCenterUv{};
		std::array<float, kMaxWarpCenters> _warpStrength{};
		std::array<float, kMaxWarpCenters> _warpInfluenceRadius{};
	};

} // namespace Engine
