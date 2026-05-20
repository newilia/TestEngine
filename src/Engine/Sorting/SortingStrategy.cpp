#include "Engine/Sorting/SortingStrategy.h"

#include "SortingStrategy.generated.hpp"

int RelativeSortingStrategy::GetSortKey() const {
	return _priority;
}

int RelativeSortingStrategy::GetPriority() const {
	return _priority;
}

void RelativeSortingStrategy::SetPriority(int priority) {
	_priority = priority;
}
