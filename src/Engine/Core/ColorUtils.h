#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Utils {

	struct HsvShapeColors
	{
		sf::Color fill;
		sf::Color outline;
	};

	/// Random hue; fill S/V near 75%; outline shares hue/S with value 25%.
	[[nodiscard]] HsvShapeColors RandomHsvShapeColors();

} // namespace Utils
