#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/Delegates.h"

#include <bitset>
#include <memory>
#include <vector>

struct IntersectionDetails;

class OverlappingBehaviour : public Behaviour
{
public:
	std::bitset<8> _overlappingGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> _overlappingCallbacks;
};
