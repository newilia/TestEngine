#include "ComponentHolder.h"

void ComponentHolderBase::addComponent(shared_ptr<ComponentBase>&& component) {
	_components.emplace_back(component);
}
