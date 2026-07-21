#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class ComposedSurfaceVisual;

namespace Engine {

	void DrawComposedSurface(const ComposedSurfaceVisual& visual, sf::RenderTarget& target, sf::RenderStates states);

} // namespace Engine
