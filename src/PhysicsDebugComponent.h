#pragma once
#include <SFML/Graphics/Drawable.hpp>

#include "ComponentBase.h"

class PhysicsDebugComponent
	: public sf::Drawable
	, public ComponentBase
{
public:
	PhysicsDebugComponent(ComponentHolderBase* node) : ComponentBase(node) {}
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};