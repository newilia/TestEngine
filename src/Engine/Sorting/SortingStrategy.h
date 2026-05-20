#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

class SortingStrategy : public EntityOnNode
{
	META_CLASS()

public:
	~SortingStrategy() override = default;

	virtual int GetSortKey() const = 0;
};

class RelativeSortingStrategy : public SortingStrategy
{
	META_CLASS()
	META_PROPERTY_BASE(SortingStrategy)

public:
	~RelativeSortingStrategy() override = default;

	int GetSortKey() const override;

public:
	int GetPriority() const;
	void SetPriority(int priority);

private:
	/// @property
	int _priority = 0;
};
