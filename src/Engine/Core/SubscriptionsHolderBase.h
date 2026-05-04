#pragma once

#include "Engine/Core/Signal.h"

#include <type_traits>
#include <utility>
#include <vector>

/// Owns `Signal<Args...>::ScopedSubscription` for one signature; `DisconnectAll()` clears (and unsubscribes).
template <class... Args>
class SubscriptionHolderBase
{
public:
	SubscriptionHolderBase() = default;
	virtual ~SubscriptionHolderBase() = default;

	SubscriptionHolderBase(const SubscriptionHolderBase&) = delete;
	SubscriptionHolderBase& operator=(const SubscriptionHolderBase&) = delete;
	SubscriptionHolderBase(SubscriptionHolderBase&&) noexcept = default;
	SubscriptionHolderBase& operator=(SubscriptionHolderBase&&) noexcept = default;

	template <class F>
	    requires std::is_invocable_v<F, Args...>
	void Subscribe(Signal<Args...>& signal, F&& callable) {
		_subscriptions.emplace_back(
		    typename Signal<Args...>::ScopedSubscription(std::move(signal.Subscribe(std::forward<F>(callable)))));
	}

	void UnsubscribeAll() {
		_subscriptions.clear();
	}

private:
	std::vector<typename Signal<Args...>::ScopedSubscription> _subscriptions;
};
