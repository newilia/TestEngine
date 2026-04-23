#pragma once
#include "ComponentBase.h"

#include <vector>

using std::shared_ptr;

class ComponentHolderBase
{
public:
	virtual ~ComponentHolderBase() = default;

	template <typename TComponent>
	shared_ptr<TComponent> FindComponent() const;

	template <typename TComponent>
	shared_ptr<TComponent> RequireComponent();

	template <typename TComponent>
	void RemoveComponent();

private:
	void AddComponent(shared_ptr<ComponentBase>&& component);
	std::vector<shared_ptr<ComponentBase>> _components;
};

template <typename TComponent>
shared_ptr<TComponent> ComponentHolderBase::FindComponent() const {
	for (auto& comp : _components) {
		if (auto typedComp = std::dynamic_pointer_cast<TComponent>(comp)) {
			return typedComp;
		}
	}
	return nullptr;
}

template <typename TComponent>
shared_ptr<TComponent> ComponentHolderBase::RequireComponent() {
	if (auto&& comp = FindComponent<TComponent>()) {
		return comp;
	}
	auto newComp = std::make_shared<TComponent>(this);
	AddComponent(newComp);
	return newComp;
}

template <typename TComponent>
void ComponentHolderBase::RemoveComponent() {
	auto it = std::find_if(_components.begin(), _components.end(),
	                       [](auto comp) { return std::dynamic_pointer_cast<TComponent>(comp) != nullptr; });
	if (it != _components.end()) {
		_components.erase(it);
	}
}

template <typename TComponent>
class ComponentHolder : public ComponentHolderBase
{
public:
	ComponentHolder() : ComponentHolderBase() { RequireComponent<TComponent>(); }
};