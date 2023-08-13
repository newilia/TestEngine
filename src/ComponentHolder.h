#pragma once
#include <vector>

#include "ComponentBase.h"

using std::shared_ptr;

class ComponentHolderBase {
public:
	virtual ~ComponentHolderBase() = default;

	template<typename TComponent>
	shared_ptr<TComponent> findComponent() const;

	template<typename TComponent>
	shared_ptr<TComponent> requireComponent();

	template<typename TComponent>
	void removeComponent();

private:
	void addComponent(shared_ptr<ComponentBase>&& component);
	std::vector<shared_ptr<ComponentBase>> mComponents;
};

template <typename TComponent>
shared_ptr<TComponent> ComponentHolderBase::findComponent() const {
	for (auto& comp : mComponents) {
		if (auto typedComp = std::dynamic_pointer_cast<TComponent>(comp)) {
			return typedComp;
		}
	}
	return nullptr;
}

template <typename TComponent>
shared_ptr<TComponent> ComponentHolderBase::requireComponent() {
	if (auto&& comp = findComponent<TComponent>()) {
		return comp;
	}
	auto newComp = std::make_shared<TComponent>(this);
	addComponent(newComp);
	return newComp;
}

template <typename TComponent>
void ComponentHolderBase::removeComponent() {
	auto it = std::find_if(mComponents.begin(), mComponents.end(), [](auto comp) {
		return std::dynamic_pointer_cast<TComponent>(comp) != nullptr;
	});
	if (it != mComponents.end()) {
		mComponents.erase(it);
	}
}

template <typename TComponent>
class ComponentHolder : public ComponentHolderBase {
public:
	ComponentHolder() : ComponentHolderBase() {
		requireComponent<TComponent>();
	}
};