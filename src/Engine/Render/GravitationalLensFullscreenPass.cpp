#include "Engine/Render/GravitationalLensFullscreenPass.h"

#include "Engine/Behaviour/GravitationalLensBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>

namespace Engine {

	namespace {

		bool SceneHasEnabledLensRecursive(const SceneNode& node) {
			if (!node.IsEnabled() || !node.IsVisible()) {
				return false;
			}
			if (auto lens = node.FindBehaviour<GravitationalLensBehaviour>()) {
				if (lens->IsLensEffectEnabled()) {
					return true;
				}
			}
			for (const auto& child : node.GetChildren()) {
				if (child && SceneHasEnabledLensRecursive(*child)) {
					return true;
				}
			}
			return false;
		}

		void CollectLensesRecursive(const std::shared_ptr<SceneNode>& node,
		                            std::vector<std::shared_ptr<GravitationalLensBehaviour>>& out) {
			if (!node || !node->IsEnabled() || !node->IsVisible()) {
				return;
			}
			if (auto lens = node->FindBehaviour<GravitationalLensBehaviour>()) {
				if (lens->IsLensEffectEnabled()) {
					out.push_back(lens);
				}
			}
			for (const auto& child : node->GetChildren()) {
				CollectLensesRecursive(child, out);
			}
		}

	} // namespace

	bool GravitationalLensFullscreenPass::IsShaderReady() const {
		return _shaderReady;
	}

	GravitationalLensFullscreenPass::GravitationalLensFullscreenPass() {
		TryLoadShader();
	}

	bool GravitationalLensFullscreenPass::TryLoadShader() {
		if (!sf::Shader::isAvailable()) {
			return false;
		}
		if (!_shader.loadFromFile("assets/shaders/GravitationalLens.frag", sf::Shader::Type::Fragment)) {
			return false;
		}
		_shaderReady = true;
		return true;
	}

	bool GravitationalLensFullscreenPass::ShouldUseEffect(const Scene& scene) const {
		if (!_shaderReady) {
			return false;
		}
		return SceneHasEnabledLensRecursive(scene);
	}

	void GravitationalLensFullscreenPass::Prepare(const std::shared_ptr<Scene>& scene,
	                                              const ViewportFullscreenPresentContext& /*ctx*/) {
		_activeLenses.clear();
		if (!scene) {
			return;
		}
		CollectLensesRecursive(scene, _activeLenses);
		if (_activeLenses.size() > kMaxLenses) {
			_activeLenses.resize(kMaxLenses);
		}
	}

	void GravitationalLensFullscreenPass::Apply(const sf::Texture& inputTexture, sf::RenderTarget& outputTarget,
	                                            const ViewportFullscreenPresentContext& ctx) {
		const sf::Vector2f quadSize(sf::Vector2f(ctx.pixelSize));
		sf::RectangleShape quad(quadSize);

		if (!_shaderReady || _activeLenses.empty()) {
			quad.setTexture(&inputTexture);
			quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));
			outputTarget.draw(quad);
			return;
		}

		const auto& window = ctx.window;
		std::size_t packedCount = 0;

		for (const auto& lens : _activeLenses) {
			auto node = lens->GetNode();
			if (!node || packedCount >= kMaxLenses) {
				continue;
			}

			const sf::Vector2f world = Utils::GetWorldPos(node);
			const sf::Vector2i px = window.mapCoordsToPixel(world, ctx.sceneView);
			const sf::Vector2f uvOff = lens->GetUvOffset();
			const float u = (static_cast<float>(px.x) + 0.5f) / static_cast<float>(ctx.pixelSize.x) + uvOff.x;
			const float pyNorm = (static_cast<float>(px.y) + 0.5f) / static_cast<float>(ctx.pixelSize.y);
			// Window pixels are top-down; fullscreen quad texture coords match SFML/OpenGL (v grows upward).
			const float v = (1.f - pyNorm) + uvOff.y;
			_lensUv[packedCount] = sf::Glsl::Vec2(u, v);
			_lensAmplitude[packedCount] = lens->GetAmplitude() * 0.1f;
			_lensFalloff[packedCount] = lens->GetFalloff() * 0.1f;
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
		_shader.setUniform("lens_count", static_cast<int>(packedCount));
		_shader.setUniformArray("lens_uv", _lensUv.data(), packedCount);
		_shader.setUniformArray("lens_amplitude", _lensAmplitude.data(), packedCount);
		_shader.setUniformArray("lens_falloff", _lensFalloff.data(), packedCount);

		quad.setTexture(&inputTexture);
		quad.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(ctx.pixelSize)));

		sf::RenderStates states;
		states.shader = &_shader;
		outputTarget.draw(quad, states);
	}

} // namespace Engine
