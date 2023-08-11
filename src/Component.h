#pragma once
#include "Node.h"

class ComponentBase : public std::enable_shared_from_this<ComponentBase> {
public:
	virtual ~ComponentBase() = default;
protected:
};
