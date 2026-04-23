#pragma once
#include "Engine/ComponentBase.h"
#include "Engine/Delegates.h"

#include <bitset>
#include <vector>

struct IntersectionDetails;
class AbstractBody;

class OverlappingComponent : public ComponentBase
{
public:
	OverlappingComponent(ComponentHolderBase* holder) : ComponentBase(holder) {}

	std::bitset<8> _overlappingGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> _overlappingCallbacks;
};
