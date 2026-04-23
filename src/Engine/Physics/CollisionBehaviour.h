#pragma once

#include "Engine/Behaviour.h"
#include "Engine/Delegates.h"

#include <bitset>
#include <memory>
#include <vector>

struct IntersectionDetails;

class CollisionBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	std::bitset<8> _collisionGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> _collisionCallbacks;
};
