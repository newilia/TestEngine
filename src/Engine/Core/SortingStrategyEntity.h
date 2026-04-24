#pragma once

#include "EntityOnNode.h"

class SortingStrategyEntity : public EntityOnNode
{
public:
	~SortingStrategyEntity() override = default;

	virtual int GetSortLayer() const { return 0; }
};
