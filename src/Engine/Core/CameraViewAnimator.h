#pragma once

#include <SFML/System/Vector2.hpp>

#include <optional>

namespace sf {
	class RenderWindow;
	class View;
} // namespace sf

namespace Engine {

	class CameraViewAnimator
	{
	public:
		[[nodiscard]] bool IsAnimating() const;

		void Cancel();

		void ApplyView(sf::RenderWindow& window, const sf::View& view);

		void RequestZoom(sf::RenderWindow& window, float zoomFactor, std::optional<sf::Vector2i> focusPixel);

		void RequestFocusCenter(sf::RenderWindow& window, sf::Vector2f worldCenter);

		void OffsetTargetCenter(sf::Vector2f worldDelta);

		void ScaleTargets(sf::Vector2f sizeScale);

		void Update(sf::RenderWindow& window, float dtSec);

	private:
		[[nodiscard]] sf::Vector2f GetBaseSizeForZoomRequest(const sf::View& currentView) const;

		sf::Vector2f _targetCenter{};
		sf::Vector2f _targetSize{};
		std::optional<sf::Vector2i> _anchorPixel;
		std::optional<sf::Vector2f> _anchorWorld;
		bool _hasTarget = false;
	};

} // namespace Engine
