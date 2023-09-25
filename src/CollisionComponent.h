#pragma once
#include "ComponentBase.h"
#include <bitset>
#include <vector>

#include "Delegates.h"

struct IntersectionDetails;
class AbstractBody;

class CollisionComponent : public ComponentBase {
public:
	CollisionComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	std::bitset<8> mCollisionGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> mCollisionCallbacks;
};
