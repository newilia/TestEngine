#pragma once

#include "EntityOnNode.h"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class NodeVisual : public EntityOnNode
{
public:
	~NodeVisual() override = default;

	virtual void Draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
};
