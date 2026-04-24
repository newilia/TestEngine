#pragma once

#include "EntityOnNode.h"

class SortingStrategy : public EntityOnNode
{
public:
	~SortingStrategy() override = default;

	virtual int GetSortLayer() const { return 0; }
};
