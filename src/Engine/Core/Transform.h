#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <SFML/Graphics/Transformable.hpp>

/// Per-node spatial transform; held by `SceneNode` and exposed like `Visual` / behaviours.
class Transform final : public EntityOnNode, public sf::Transformable
{
public:
	void BuildPropertyTree(Engine::PropertyBuilder& b) override;
};
