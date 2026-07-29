#pragma once

// Multicast signal with explicit unsubscribe handles. Single-threaded (expected: main / game thread).
// Subscribers are stored as IDelegate instances; use createDelegate(...) for weak bindings to shared owners.
// Subscriptions hold a weak_ptr to shared slot storage so Unsubscribe() is safe after the Signal object is destroyed.

#include "Engine/Core/Delegates.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template <class... Args>
class Signal
{
public:
	class Subscription;
	friend class Subscription;

private:
	struct State;

public:
	class Subscription
	{
	public:
		Subscription() = default;

		void Unsubscribe() {
			if (_id != 0) {
				if (const auto state = _state.lock()) {
					state->RemoveSlot(_id);
				}
				_state.reset();
				_id = 0;
			}
		}

		[[nodiscard]] explicit operator bool() const {
			return _id != 0 && !_state.expired();
		}

		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;

		Subscription(Subscription&& o) noexcept : _state(std::move(o._state)), _id(o._id) {
			o._id = 0;
		}

		Subscription& operator=(Subscription&& o) noexcept {
			if (this != &o) {
				_state = std::move(o._state);
				_id = o._id;
				o._id = 0;
			}
			return *this;
		}

	private:
		friend class Signal<Args...>;

		Subscription(std::weak_ptr<State> state, std::uint64_t id) : _state(std::move(state)), _id(id) {}

		std::weak_ptr<State> _state;
		std::uint64_t _id{};
	};

	/// Unsubscribes in the destructor (move-only).
	class ScopedSubscription
	{
	public:
		ScopedSubscription() = default;

		explicit ScopedSubscription(Subscription&& c) : _conn(std::move(c)) {}

		~ScopedSubscription() {
			_conn.Unsubscribe();
		}

		ScopedSubscription(const ScopedSubscription&) = delete;
		ScopedSubscription& operator=(const ScopedSubscription&) = delete;

		ScopedSubscription(ScopedSubscription&&) noexcept = default;

		ScopedSubscription& operator=(ScopedSubscription&& o) noexcept {
			if (this != &o) {
				_conn.Unsubscribe();
				_conn = std::move(o._conn);
			}
			return *this;
		}

		[[nodiscard]] Subscription Release() {
			Subscription out = std::move(_conn);
			return out;
		}

	private:
		Subscription _conn;
	};

	using DelegatePtr = std::unique_ptr<IDelegate<Args...>>;

	Signal() = default;
	Signal(const Signal&) = delete;
	Signal& operator=(const Signal&) = delete;
	Signal(Signal&&) = delete;
	Signal& operator=(Signal&&) = delete;

	[[nodiscard]] Subscription Subscribe(DelegatePtr&& delegate) {
		const std::uint64_t id = _state->nextId++;
		_state->slots.push_back(Slot{id, std::move(delegate)});
		return Subscription(_state, id);
	}

	[[nodiscard]] Subscription Subscribe(std::function<void(Args...)> func) {
		return Subscribe(std::make_unique<FunctionDelegate<Args...>>(std::move(func)));
	}

	template <class F>
	    requires std::is_invocable_v<F, Args...>
	[[nodiscard]] Subscription Subscribe(F&& callable) {
		return Subscribe(std::function<void(Args...)>(std::forward<F>(callable)));
	}

	template <class... UArgs>
	void Emit(UArgs&&... args) {
		std::vector<IDelegate<Args...>*> snapshot;
		snapshot.reserve(_state->slots.size());
		for (auto& slot : _state->slots) {
			if (!slot.delegate->expired()) {
				snapshot.push_back(slot.delegate.get());
			}
		}

		for (IDelegate<Args...>* del : snapshot) {
			(*del)(std::forward<UArgs>(args)...);
		}
	}

	template <class... UArgs>
	void operator()(UArgs&&... args) {
		Emit(std::forward<UArgs>(args)...);
	}

	[[nodiscard]] std::size_t SubscriberCount() const {
		return _state->slots.size();
	}

private:
	struct Slot
	{
		std::uint64_t id{};
		DelegatePtr delegate;
	};

	struct State
	{
		std::vector<Slot> slots;
		std::uint64_t nextId = 1;

		void RemoveSlot(std::uint64_t id) {
			const auto it = std::find_if(slots.begin(), slots.end(), [id](const Slot& s) {
				return s.id == id;
			});
			if (it != slots.end()) {
				slots.erase(it);
			}
		}
	};

	void RemoveExpiredSlots() {
		_state->slots.erase(std::remove_if(_state->slots.begin(), _state->slots.end(),
		                        [](const Slot& s) {
			                        return s.delegate->expired();
		                        }),
		    _state->slots.end());
	}

	std::shared_ptr<State> _state = std::make_shared<State>();
};
