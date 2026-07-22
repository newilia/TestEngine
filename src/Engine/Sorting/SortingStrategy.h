#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

META_ENUM(SortingStrategyType, Relative, Absolute);

class SortingStrategy : public EntityOnNode
{
	META_CLASS()

public:
	~SortingStrategy() override = default;

	virtual int GetSortKey() const;
	virtual void SetSortKey(int sortKey);
	virtual SortingStrategyType GetType() const;
	virtual void SetType(SortingStrategyType type);

private:
	/// @property
	int _sortKey = 0;
	/// @property
	SortingStrategyType _type = SortingStrategyType::Relative;
};

// Temp dummy, TODO refactor serialization and deserialization
class DefaultSortingStrategy : public SortingStrategy
{
	META_CLASS()
	META_PROPERTY_BASE(SortingStrategy)

public:
	~DefaultSortingStrategy() override = default;
};
