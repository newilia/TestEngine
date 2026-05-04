#pragma once

#include "Engine/Core/Signal.h"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

/// Owns `Signal<...>::ScopedSubscription` instances of any signature; `UnsubscribeAll()` clears them.
class SubscriptionsHolderBase
{
public:
	SubscriptionsHolderBase() = default;
	virtual ~SubscriptionsHolderBase() = default;

	SubscriptionsHolderBase(const SubscriptionsHolderBase&) = delete;
	SubscriptionsHolderBase& operator=(const SubscriptionsHolderBase&) = delete;
	SubscriptionsHolderBase(SubscriptionsHolderBase&&) noexcept = default;
	SubscriptionsHolderBase& operator=(SubscriptionsHolderBase&&) noexcept = default;

	template <class... Args, class F>
	    requires std::is_invocable_v<F, Args...>
	void Subscribe(Signal<Args...>& signal, F&& callable) {
		_slots.push_back(std::make_unique<Slot<Args...>>(
		    typename Signal<Args...>::ScopedSubscription(signal.Subscribe(std::forward<F>(callable)))));
	}

	void UnsubscribeAll() {
		_slots.clear();
	}

private:
	struct ISlot
	{
		virtual ~ISlot() = default;
	};

	template <class... Args>
	struct Slot final : ISlot
	{
		explicit Slot(typename Signal<Args...>::ScopedSubscription&& s) : _scoped(std::move(s)) {}

		typename Signal<Args...>::ScopedSubscription _scoped;
	};

	std::vector<std::unique_ptr<ISlot>> _slots;
};
