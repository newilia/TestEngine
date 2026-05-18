#pragma once

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

class ShapeVisualBase;

namespace Engine {

	/// Second draw pass: adds shape lighting on top of an already drawn base fill (see `ShapeVisualBase::Draw`).
	void DrawShapeLightingPass(const ShapeVisualBase& visual, sf::RenderTarget& target, sf::RenderStates states);

} // namespace Engine
