#pragma once

#include "Engine/Core/Behaviour.h"
#include "Engine/Core/Delegates.h"

#include <bitset>
#include <memory>
#include <vector>

struct IntersectionDetails;

class OverlappingBehaviour : public Behaviour
{
public:
	void OnAttached() override {}

	std::bitset<8> _overlappingGroups;
	std::vector<std::shared_ptr<IDelegate<const IntersectionDetails&>>> _overlappingCallbacks;
};
