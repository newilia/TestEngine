#pragma once
#include <memory>
class ComponentHolderBase;

class ComponentBase : public std::enable_shared_from_this<ComponentBase> {
public:
	ComponentBase(ComponentHolderBase* holder) { mHolder = holder; }
	virtual ~ComponentBase() = default;
protected:
	ComponentHolderBase* mHolder = nullptr;
};
