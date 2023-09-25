#pragma once
#include "ComponentBase.h"
#include <bitset>
#include <vector>

#include "Delegates.h"

struct IntersectionDetails;
class AbstractBody;

class OverlappingComponent : public ComponentBase {
public:
	OverlappingComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}
	std::bitset<8> mOverlappingGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> mOverlappingCallbacks;
};
