#pragma once

#include "Engine/Core/EntityOnNode.h"

class SortingStrategy : public EntityOnNode
{
public:
	~SortingStrategy() override = default;

	virtual int GetSortLayer() const { return 0; }
};
