#include "Engine/Render/RadialUvWarpFullscreenPass.h"

#include "Engine/Behaviour/RadialUvWarpBehaviour.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cmath>

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
		                                   std::vector<std::shared_ptr<RadialUvWarpBehaviour>>& warps) {
			if (!node || !node->IsEnabled() || !node->IsVisible()) {
				return;
			}
			if (auto warp = node->FindBehaviour<RadialUvWarpBehaviour>()) {
				if (warp->IsRadialUvWarpEnabled()) {
					warps.push_back(warp);
				}
			}
			for (const auto& child : node->GetChildren()) {
				CollectRadialUvWarpsRecursive(child, warps);
			}
		}

		sf::Vector2f WarpPixelToUv(const sf::Vector2i& px, sf::Vector2u pixelSize, const sf::Vector2f& uvOffset) {
			const float u = (static_cast<float>(px.x) + 0.5f) / static_cast<float>(pixelSize.x) + uvOffset.x;
			const float pyNorm = (static_cast<float>(px.y) + 0.5f) / static_cast<float>(pixelSize.y);
			const float v = (1.f - pyNorm) + uvOffset.y;
			return {u, v};
		}

		float ShaderDistUv(const sf::Vector2f& a, const sf::Vector2f& b, float aspectRatio) {
			const float dx = (a.x - b.x) * aspectRatio;
			const float dy = a.y - b.y;
			return std::hypot(dx, dy);
		}

		// 1-pixel step: world-space eps + mapCoordsToPixel often share one pixel → zero UV derivative (flicker).
		float WorldRadiusToShaderDist(const sf::RenderWindow& window, const sf::View& sceneView, sf::Vector2u pixelSize,
		                              const sf::Vector2f& uvOffset, float aspectRatio, sf::Vector2i pxAnchor) {
			const int pw = static_cast<int>(pixelSize.x);
			const int ph = static_cast<int>(pixelSize.y);
			if (pw <= 1 || ph <= 1) {
				const float sx = std::max(sceneView.getSize().x, 1e-4f);
				const float sy = std::max(sceneView.getSize().y, 1e-4f);
				return std::max(1.f / std::min(sx, sy), 1e-10f);
			}

			const int x0 = std::clamp(pxAnchor.x, 0, pw - 1);
			const int y0 = std::clamp(pxAnchor.y, 0, ph - 1);
			int x1 = x0 + 1;
			if (x1 >= pw) {
				x1 = x0 - 1;
			}
			int y1 = y0 + 1;
			if (y1 >= ph) {
				y1 = y0 - 1;
			}
			if (x1 == x0 || y1 == y0) {
				const float sx = std::max(sceneView.getSize().x, 1e-4f);
				const float sy = std::max(sceneView.getSize().y, 1e-4f);
				return std::max(1.f / std::min(sx, sy), 1e-10f);
			}

			const sf::Vector2f uv0 = WarpPixelToUv({x0, y0}, pixelSize, uvOffset);
			const sf::Vector2f uvX = WarpPixelToUv({x1, y0}, pixelSize, uvOffset);
			const sf::Vector2f uvY = WarpPixelToUv({x0, y1}, pixelSize, uvOffset);

			const sf::Vector2f w0 = window.mapPixelToCoords(sf::Vector2i(x0, y0), sceneView);
			const sf::Vector2f wx = window.mapPixelToCoords(sf::Vector2i(x1, y0), sceneView);
			const sf::Vector2f wy = window.mapPixelToCoords(sf::Vector2i(x0, y1), sceneView);

			const float dMetricDx = ShaderDistUv(uvX, uv0, aspectRatio);
			const float dMetricDy = ShaderDistUv(uvY, uv0, aspectRatio);
			const float dWorldX = std::hypot(wx.x - w0.x, wx.y - w0.y);
			const float dWorldY = std::hypot(wy.x - w0.x, wy.y - w0.y);

			const float scaleX = dWorldX > 1e-20f ? dMetricDx / dWorldX : 0.f;
			const float scaleY = dWorldY > 1e-20f ? dMetricDy / dWorldY : 0.f;
			float scale = 0.5f * (scaleX + scaleY);
			if (!std::isfinite(scale) || scale < 1e-20f) {
				const float sx = std::max(sceneView.getSize().x, 1e-4f);
				const float sy = std::max(sceneView.getSize().y, 1e-4f);
				scale = 1.f / std::min(sx, sy);
			}
			return std::max(scale, 1e-10f);
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
		const auto root = scene.GetRoot();
		if (!root) {
			return false;
		}
		return SceneHasEnabledRadialUvWarpRecursive(*root);
	}

	void RadialUvWarpFullscreenPass::Prepare(const std::shared_ptr<Scene>& scene,
	                                         const ViewportFullscreenPresentContext& /*ctx*/) {
		_activeWarps.clear();
		if (!scene) {
			return;
		}
		const auto root = scene->GetRoot();
		if (!root) {
			return;
		}
		CollectRadialUvWarpsRecursive(root, _activeWarps);
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

		const float pxW = std::max(static_cast<float>(ctx.pixelSize.x), 1.f);
		const float pxH = std::max(static_cast<float>(ctx.pixelSize.y), 1.f);
		const float aspectRatio = pxW / pxH;

		for (const auto& warp : _activeWarps) {
			const auto anchor = warp->GetNode();
			if (!anchor || packedCount >= kMaxWarpCenters) {
				continue;
			}

			const sf::Vector2f world = Utils::GetWorldPos(anchor);
			const sf::Vector2i px = window.mapCoordsToPixel(world, ctx.sceneView);
			const sf::Vector2f uvOff = warp->GetUvOffset();
			const sf::Vector2f uv = WarpPixelToUv(px, ctx.pixelSize, uvOff);
			_warpCenterUv[packedCount] = sf::Glsl::Vec2(uv.x, uv.y);
			_warpStrength[packedCount] = warp->GetWarpStrength();
			const float worldToShader =
			    WorldRadiusToShaderDist(window, ctx.sceneView, ctx.pixelSize, uvOff, aspectRatio, px);
			_warpInfluenceRadius[packedCount] = warp->GetInfluenceRadius() * worldToShader;
			++packedCount;
		}

		if (packedCount == 0) {
			quad.setTexture(&inputTexture);
			quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));
			outputTarget.draw(quad);
			return;
		}

		_shader.setUniform("aspect_ratio", aspectRatio);
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
