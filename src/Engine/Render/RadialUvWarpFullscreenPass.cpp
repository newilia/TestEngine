#include "Engine/Render/RadialUvWarpFullscreenPass.h"

#include "Engine/Behaviour/RadialUvWarpBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>

namespace Engine {

	namespace {

		bool SceneHasEnabledRadialUvWarpRecursive(const SceneNode& node) {
			if (!node.IsEnabled() || !node.IsVisible()) {
				return false;
			}
			if (auto warp = node.FindBehaviour<RadialUvWarpBehaviour>()) {
				if (warp->IsRadialUvWarpEnabled()) {
					return true;
				}
			}
			for (const auto& child : node.GetChildren()) {
				if (child && SceneHasEnabledRadialUvWarpRecursive(*child)) {
					return true;
				}
			}
			return false;
		}

		void CollectRadialUvWarpsRecursive(const std::shared_ptr<SceneNode>& node,
		                                   std::vector<std::shared_ptr<RadialUvWarpBehaviour>>& out) {
			if (!node || !node->IsEnabled() || !node->IsVisible()) {
				return;
			}
			if (auto warp = node->FindBehaviour<RadialUvWarpBehaviour>()) {
				if (warp->IsRadialUvWarpEnabled()) {
					out.push_back(warp);
				}
			}
			for (const auto& child : node->GetChildren()) {
				CollectRadialUvWarpsRecursive(child, out);
			}
		}

	} // namespace

	bool RadialUvWarpFullscreenPass::IsShaderReady() const {
		return _shaderReady;
	}

	RadialUvWarpFullscreenPass::RadialUvWarpFullscreenPass() {
		TryLoadShader();
	}

	bool RadialUvWarpFullscreenPass::TryLoadShader() {
		if (!sf::Shader::isAvailable()) {
			return false;
		}
		if (!_shader.loadFromFile("assets/shaders/RadialUvWarp.frag", sf::Shader::Type::Fragment)) {
			return false;
		}
		_shaderReady = true;
		return true;
	}

	bool RadialUvWarpFullscreenPass::ShouldUseEffect(const Scene& scene) const {
		if (!_shaderReady) {
			return false;
		}
		return SceneHasEnabledRadialUvWarpRecursive(scene);
	}

	void RadialUvWarpFullscreenPass::Prepare(const std::shared_ptr<Scene>& scene,
	                                         const ViewportFullscreenPresentContext& /*ctx*/) {
		_activeWarps.clear();
		if (!scene) {
			return;
		}
		CollectRadialUvWarpsRecursive(scene, _activeWarps);
		if (_activeWarps.size() > kMaxWarpCenters) {
			_activeWarps.resize(kMaxWarpCenters);
		}
	}

	void RadialUvWarpFullscreenPass::Apply(const sf::Texture& inputTexture, sf::RenderTarget& outputTarget,
	                                       const ViewportFullscreenPresentContext& ctx) {
		const sf::Vector2f quadSize(sf::Vector2f(ctx.pixelSize));
		sf::RectangleShape quad(quadSize);

		if (!_shaderReady || _activeWarps.empty()) {
			quad.setTexture(&inputTexture);
			quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));
			outputTarget.draw(quad);
			return;
		}

		const auto& window = ctx.window;
		std::size_t packedCount = 0;

		for (const auto& warp : _activeWarps) {
			auto node = warp->GetNode();
			if (!node || packedCount >= kMaxWarpCenters) {
				continue;
			}

			const sf::Vector2f world = Utils::GetWorldPos(node);
			const sf::Vector2i px = window.mapCoordsToPixel(world, ctx.sceneView);
			const sf::Vector2f uvOff = warp->GetUvOffset();
			const float u = (static_cast<float>(px.x) + 0.5f) / static_cast<float>(ctx.pixelSize.x) + uvOff.x;
			const float pyNorm = (static_cast<float>(px.y) + 0.5f) / static_cast<float>(ctx.pixelSize.y);
			const float v = (1.f - pyNorm) + uvOff.y;
			_warpCenterUv[packedCount] = sf::Glsl::Vec2(u, v);
			_warpStrength[packedCount] = warp->GetWarpStrength();
			_warpInfluenceRadius[packedCount] = warp->GetInfluenceRadius();
			++packedCount;
		}

		if (packedCount == 0) {
			quad.setTexture(&inputTexture);
			quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));
			outputTarget.draw(quad);
			return;
		}

		const float w = std::max(static_cast<float>(ctx.pixelSize.x), 1.f);
		const float h = std::max(static_cast<float>(ctx.pixelSize.y), 1.f);
		_shader.setUniform("aspect_ratio", w / h);

		_shader.setUniform("texture", sf::Shader::CurrentTexture);
		_shader.setUniform("warp_count", static_cast<int>(packedCount));
		_shader.setUniformArray("warp_center_uv", _warpCenterUv.data(), packedCount);
		_shader.setUniformArray("warp_strength", _warpStrength.data(), packedCount);
		_shader.setUniformArray("warp_influence_radius", _warpInfluenceRadius.data(), packedCount);

		quad.setTexture(&inputTexture);
		quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));

		sf::RenderStates states;
		states.shader = &_shader;
		outputTarget.draw(quad, states);
	}

} // namespace Engine
