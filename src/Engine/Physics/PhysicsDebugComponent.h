#pragma once
#include "Engine/ComponentBase.h"

#include <SFML/Graphics/Drawable.hpp>

class PhysicsDebugComponent : public sf::Drawable, public ComponentBase
{
public:
	PhysicsDebugComponent(ComponentHolderBase* node) : ComponentBase(node) {}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};