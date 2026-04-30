#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/Signal.h"

#include <bitset>

struct IntersectionDetails;

class OverlappingBehaviour : public Behaviour
{
public:
	std::bitset<8> _overlappingGroups;
	Signal<const IntersectionDetails&> _overlappingCallbacks;
};
