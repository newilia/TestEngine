#pragma once

// Multicast signal with explicit unsubscribe handles. Single-threaded (expected: main / game thread).
// Subscribers are stored as IDelegate instances; use createDelegate(...) for weak bindings to shared owners.

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

	class Subscription
	{
	public:
		Subscription() = default;

		void Unsubscribe() {
			if (_signal && _id != 0) {
				_signal->RemoveSlot(_id);
				_signal = nullptr;
				_id = 0;
			}
		}

		[[nodiscard]] explicit operator bool() const {
			return _signal != nullptr && _id != 0;
		}

		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;

		Subscription(Subscription&& o) noexcept : _signal(o._signal), _id(o._id) {
			o._signal = nullptr;
			o._id = 0;
		}

		Subscription& operator=(Subscription&& o) noexcept {
			if (this != &o) {
				_signal = o._signal;
				_id = o._id;
				o._signal = nullptr;
				o._id = 0;
			}
			return *this;
		}

	private:
		friend class Signal<Args...>;

		Subscription(Signal* sig, std::uint64_t id) : _signal(sig), _id(id) {}

		Signal* _signal{};
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
		const std::uint64_t id = _nextId++;
		_slots.push_back(Slot{id, std::move(delegate)});
		return Subscription(this, id);
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
		RemoveExpiredSlots();

		std::vector<IDelegate<Args...>*> snapshot;
		snapshot.reserve(_slots.size());
		for (auto& slot : _slots) {
			if (!slot.delegate->expired()) {
				snapshot.push_back(slot.delegate.get());
			}
		}

		for (IDelegate<Args...>* del : snapshot) {
			(*del)(std::forward<UArgs>(args)...);
		}

		RemoveExpiredSlots();
	}

	template <class... UArgs>
	void operator()(UArgs&&... args) {
		Emit(std::forward<UArgs>(args)...);
	}

	[[nodiscard]] std::size_t SubscriberCount() const {
		return _slots.size();
	}

private:
	void RemoveSlot(std::uint64_t id) {
		const auto it = std::find_if(_slots.begin(), _slots.end(), [id](const Slot& s) {
			return s.id == id;
		});
		if (it != _slots.end()) {
			_slots.erase(it);
		}
	}

	void RemoveExpiredSlots() {
		_slots.erase(std::remove_if(_slots.begin(), _slots.end(),
		                            [](const Slot& s) {
			                            return s.delegate->expired();
		                            }),
		             _slots.end());
	}

	struct Slot
	{
		std::uint64_t id{};
		DelegatePtr delegate;
	};

	std::vector<Slot> _slots;
	std::uint64_t _nextId = 1;
};
