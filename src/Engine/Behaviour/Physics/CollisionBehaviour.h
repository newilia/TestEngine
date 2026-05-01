#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"
#include "Engine/Core/Signal.h"

#include <bitset>

struct IntersectionDetails;

// TODO move to base collider
class CollisionBehaviour : public Behaviour
{
	META_CLASS()
public:
	static constexpr int CollisionGroupsCount = 8;
	// TODO support @property for bitset with custom editor widget (checkboxes for each bit)
	std::bitset<CollisionGroupsCount> _collisionGroups;
	Signal<const IntersectionDetails&> _collisionCallbacks;
};
