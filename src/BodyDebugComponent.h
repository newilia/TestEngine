#pragma once
#include <SFML/Graphics/Drawable.hpp>

#include "ComponentBase.h"

class BodyDebugComponent
	: public sf::Drawable
	, public ComponentBase
{
public:
	BodyDebugComponent(ComponentHolderBase* node) : ComponentBase(node) {}
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};