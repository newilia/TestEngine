#include "Engine/Sorting/SortingStrategy.h"

#include "SortingStrategy.generated.hpp"

int SortingStrategy::GetSortKey() const {
	return _sortKey;
}

SortingStrategyType SortingStrategy::GetType() const {
	return _type;
}

void SortingStrategy::SetSortKey(int sortKey) {
	_sortKey = sortKey;
}

void SortingStrategy::SetType(SortingStrategyType type) {
	_type = type;
}
