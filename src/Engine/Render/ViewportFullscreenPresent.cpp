#include "Engine/Render/RadialUvWarpFullscreenPass.h"
#include "Engine/Render/ViewportFullscreenEffect.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <vector>

namespace Engine {
	namespace {
		sf::RenderTexture sCaptureRt;
		sf::RenderTexture sPingRt;
		sf::Vector2u sAllocatedSize{};

		bool EnsureFullscreenRenderTextures(sf::Vector2u size) {
			if (size.x == 0u || size.y == 0u) {
				return false;
			}
			if (size == sAllocatedSize) {
				return true;
			}
			if (!sCaptureRt.resize(size)) {
				return false;
			}
			if (!sPingRt.resize(size)) {
				return false;
			}
			sAllocatedSize = size;
			sCaptureRt.setSmooth(true);
			sPingRt.setSmooth(true);
			return true;
		}
	} // namespace

	void PresentSceneWithFullscreenEffects(sf::RenderWindow& window, Scene& scene,
	                                       const std::vector<IViewportFullscreenEffect*>& effectChain) {
		if (effectChain.empty()) {
			window.draw(scene);
			return;
		}

		const sf::Vector2u pixelSize = window.getSize();
		if (!EnsureFullscreenRenderTextures(pixelSize)) {
			window.draw(scene);
			return;
		}

		const sf::View savedView = window.getView();
		ViewportFullscreenPresentContext ctx{window, pixelSize, savedView};

		const auto scenePtr = std::static_pointer_cast<Scene>(scene.shared_from_this());
		for (IViewportFullscreenEffect* effect : effectChain) {
			effect->Prepare(scenePtr, ctx);
		}

		sCaptureRt.setView(savedView);
		sCaptureRt.clear();
		sCaptureRt.draw(scene);
		sCaptureRt.display();

		const sf::Texture* currentTex = &sCaptureRt.getTexture();
		sf::RenderTexture* dstRt = &sPingRt;

		const sf::View pixelView(sf::FloatRect({0.f, 0.f}, sf::Vector2f(pixelSize)));

		for (std::size_t i = 0; i < effectChain.size(); ++i) {
			const bool isLast = (i + 1u == effectChain.size());
			if (isLast) {
				window.setView(pixelView);
				effectChain[i]->Apply(*currentTex, window, ctx);
				window.setView(savedView);
			}
			else {
				dstRt->setView(pixelView);
				dstRt->clear();
				effectChain[i]->Apply(*currentTex, *dstRt, ctx);
				dstRt->display();
				currentTex = &dstRt->getTexture();
				dstRt = (dstRt == &sPingRt) ? &sCaptureRt : &sPingRt;
			}
		}
	}

	void PresentMainWindowScene(sf::RenderWindow& window, Scene& scene) {
		std::vector<IViewportFullscreenEffect*> chain;
		auto& radialUvWarpPass = RadialUvWarpFullscreenPass::GetInstance();

		if (radialUvWarpPass.ShouldUseEffect(scene)) {
			chain.push_back(&radialUvWarpPass);
		}
		PresentSceneWithFullscreenEffects(window, scene, chain);
	}

} // namespace Engine
