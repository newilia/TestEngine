#pragma once

#include "Engine/Core/Scene.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>

namespace Engine {

	struct ViewportFullscreenPresentContext
	{
		sf::RenderWindow& window;
		sf::Vector2u pixelSize{};
		sf::View sceneView{};
	};

	class IViewportFullscreenEffect
	{
	public:
		virtual ~IViewportFullscreenEffect() = default;

		virtual void Prepare(const std::shared_ptr<Scene>& scene, const ViewportFullscreenPresentContext& ctx) = 0;

		virtual void Apply(const sf::Texture& inputTexture, sf::RenderTarget& outputTarget,
		                   const ViewportFullscreenPresentContext& ctx) = 0;
	};

	void PresentSceneWithFullscreenEffects(sf::RenderWindow& window, Scene& scene,
	                                       const std::vector<IViewportFullscreenEffect*>& effectChain);

	/// Resolves which fullscreen effects apply to the scene and presents it (or draws directly).
	void PresentMainWindowScene(sf::RenderWindow& window, Scene& scene);

} // namespace Engine
