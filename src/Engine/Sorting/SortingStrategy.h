#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

class RelativeSortingStrategy : public EntityOnNode
{
	META_CLASS()

public:
	~RelativeSortingStrategy() override = default;
	int GetPriority() const;
	void SetPriority(int priority);

private:
	/// @property
	int _priority = 0;
};
