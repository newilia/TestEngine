#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class ShapeVisualBase;

namespace Engine {

	/// If the node has `ShapeLightReceiverBehaviour` and lighting shaders load, draws with the lighting
	/// shader and returns true. Otherwise returns false (caller draws unlit).
	bool TryDrawShapeWithLighting(const ShapeVisualBase& visual, sf::RenderTarget& target, sf::RenderStates states);

} // namespace Engine
