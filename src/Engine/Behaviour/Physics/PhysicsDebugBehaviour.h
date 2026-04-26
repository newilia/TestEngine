#pragma once

#include "Engine/Behaviour/Behaviour.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class PhysicsDebugBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	void DebugDraw(sf::RenderTarget& target, sf::RenderStates states) const; // todo move out of behaviour?
};
