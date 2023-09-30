#pragma once
#include "Engine/ComponentBase.h"
#include <bitset>
#include <vector>

#include "Engine/Delegates.h"

struct IntersectionDetails;
class AbstractBody;

class CollisionComponent : public ComponentBase {
public:
	CollisionComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	std::bitset<8> mCollisionGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> mCollisionCallbacks;
};
