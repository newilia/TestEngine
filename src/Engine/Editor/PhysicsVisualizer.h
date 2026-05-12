#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf {
	class RenderWindow;
}

namespace Engine {

	class PhysicsVisualizer
	{
	public:
		void Draw(sf::RenderWindow& window) const;

		bool IsVelocityVisible() const;
		void SetVelocityVisible(bool visible);

		bool IsForceVisible() const;
		void SetForceVisible(bool visible);

		float GetVelocityScale() const;
		void SetVelocityScale(float scale);

		float GetForceScale() const;
		void SetForceScale(float scale);

		sf::Color GetVelocityColor() const;
		void SetVelocityColor(sf::Color color);

		sf::Color GetForceColor() const;
		void SetForceColor(sf::Color color);

	private:
		bool _isVelocityVisible = false;
		bool _isForceVisible = false;
		float _velocityScale = 0.5f;
		float _forceScale = 0.5f;
		sf::Color _velocityColor{100, 220, 120};
		sf::Color _forceColor{255, 160, 60};
	};

} // namespace Engine
