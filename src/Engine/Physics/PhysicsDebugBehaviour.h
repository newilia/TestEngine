#pragma once

#include "Engine/Behaviour.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class PhysicsDebugBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	void DrawDebug(sf::RenderTarget& target, sf::RenderStates states) const;
};
