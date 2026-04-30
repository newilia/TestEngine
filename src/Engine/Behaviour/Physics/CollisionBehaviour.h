#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/Signal.h"

#include <bitset>

struct IntersectionDetails;

class CollisionBehaviour : public Behaviour
{
public:
	std::bitset<8> _collisionGroups;
	Signal<const IntersectionDetails&> _collisionCallbacks;
};
