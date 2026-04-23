#pragma once
#include "Engine/ComponentBase.h"
#include "Engine/Delegates.h"

#include <bitset>
#include <vector>

struct IntersectionDetails;
class AbstractBody;

class CollisionComponent : public ComponentBase
{
public:
	CollisionComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	std::bitset<8> _collisionGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> _collisionCallbacks;
};
