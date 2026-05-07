#pragma once

#include "Engine/Core/Singleton.h"
#include "Engine/Render/ViewportFullscreenEffect.h"

#include <SFML/Graphics/Shader.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

class GravitationalLensBehaviour;

namespace Engine {

	class GravitationalLensFullscreenPass final : public IViewportFullscreenEffect,
	                                              public Singleton<GravitationalLensFullscreenPass>
	{
		friend class Singleton<GravitationalLensFullscreenPass>;

	public:
		static constexpr std::size_t kMaxLenses = 32u;

		bool IsShaderReady() const;
		bool ShouldUseEffect(const Scene& scene) const;
		void Prepare(const std::shared_ptr<Scene>& scene, const ViewportFullscreenPresentContext& ctx) override;
		void Apply(const sf::Texture& inputTexture, sf::RenderTarget& outputTarget,
		           const ViewportFullscreenPresentContext& ctx) override;

	private:
		GravitationalLensFullscreenPass();
		bool TryLoadShader();

	private:
		bool _shaderReady = false;
		sf::Shader _shader{};
		std::vector<std::shared_ptr<GravitationalLensBehaviour>> _activeLenses{};
		std::array<sf::Glsl::Vec2, kMaxLenses> _lensUv{};
		std::array<float, kMaxLenses> _lensAmplitude{};
		std::array<float, kMaxLenses> _lensFalloff{};
	};

} // namespace Engine
