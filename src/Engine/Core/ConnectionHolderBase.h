#pragma once

#include "Engine/Core/Signal.h"

#include <type_traits>
#include <utility>
#include <vector>

/// Owns `Signal<Args...>::ScopedConnection` for one signature; `DisconnectAll()` clears (and unsubscribes).
template <class... Args>
class ConnectionHolderBase
{
public:
	ConnectionHolderBase() = default;
	virtual ~ConnectionHolderBase() = default;

	ConnectionHolderBase(const ConnectionHolderBase&) = delete;
	ConnectionHolderBase& operator=(const ConnectionHolderBase&) = delete;
	ConnectionHolderBase(ConnectionHolderBase&&) noexcept = default;
	ConnectionHolderBase& operator=(ConnectionHolderBase&&) noexcept = default;

	template <class F>
	    requires std::is_invocable_v<F, Args...>
	void Connect(Signal<Args...>& signal, F&& callable) {
		_connections.emplace_back(
		    typename Signal<Args...>::ScopedConnection(std::move(signal.Connect(std::forward<F>(callable)))));
	}

	void DisconnectAll() {
		_connections.clear();
	}

private:
	std::vector<typename Signal<Args...>::ScopedConnection> _connections;
};
