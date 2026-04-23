#include "ComponentHolder.h"

void ComponentHolderBase::AddComponent(shared_ptr<ComponentBase>&& component) {
	_components.emplace_back(component);
}
