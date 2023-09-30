#include "ComponentHolder.h"


void ComponentHolderBase::addComponent(shared_ptr<ComponentBase>&& component) {
	mComponents.emplace_back(component);
}
