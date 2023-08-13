#pragma once
#include <SFML/Graphics/Drawable.hpp>

#include "ComponentBase.h"

class DebugComponent
	: public sf::Drawable
	, public ComponentBase
{
public:
	DebugComponent(ComponentHolderBase* node) : ComponentBase(node) {}
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};