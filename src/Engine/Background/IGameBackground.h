#pragma once

#include "Engine/Core/IPropertiesProvider.h"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Time.hpp>

namespace sf {
	class RenderWindow;
} // namespace sf

/// World- or view-space backdrop drawn before the scene graph each frame.
class IGameBackground : public sf::Drawable, public Engine::IPropertiesProvider
{
public:
	~IGameBackground() override = default;

	virtual void Update(const sf::RenderWindow& window, sf::Time dt) = 0;
};
