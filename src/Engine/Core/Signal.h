#pragma once

// Multicast signal with explicit disconnect handles. Single-threaded (expected: main / game thread).
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
	class Connection; // grant friendship before member definitions use private Signal API

	friend class Connection;

	class Connection
	{
	public:
		Connection() = default;

		void Disconnect() {
			if (_signal && _id != 0) {
				_signal->RemoveSlot(_id);
				_signal = nullptr;
				_id = 0;
			}
		}

		[[nodiscard]] explicit operator bool() const {
			return _signal != nullptr && _id != 0;
		}

		Connection(const Connection&) = delete;
		Connection& operator=(const Connection&) = delete;

		Connection(Connection&& o) noexcept : _signal(o._signal), _id(o._id) {
			o._signal = nullptr;
			o._id = 0;
		}

		Connection& operator=(Connection&& o) noexcept {
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

		Connection(Signal* sig, std::uint64_t id) : _signal(sig), _id(id) {}

		Signal* _signal{};
		std::uint64_t _id{};
	};

	/// Unsubscribes in the destructor (move-only).
	class ScopedConnection
	{
	public:
		ScopedConnection() = default;

		explicit ScopedConnection(Connection&& c) : _conn(std::move(c)) {}

		~ScopedConnection() {
			_conn.Disconnect();
		}

		ScopedConnection(const ScopedConnection&) = delete;
		ScopedConnection& operator=(const ScopedConnection&) = delete;

		ScopedConnection(ScopedConnection&&) noexcept = default;

		ScopedConnection& operator=(ScopedConnection&& o) noexcept {
			if (this != &o) {
				_conn.Disconnect();
				_conn = std::move(o._conn);
			}
			return *this;
		}

		[[nodiscard]] Connection Release() {
			Connection out = std::move(_conn);
			return out;
		}

	private:
		Connection _conn;
	};

	using DelegatePtr = std::unique_ptr<IDelegate<Args...>>;

	Signal() = default;
	Signal(const Signal&) = delete;
	Signal& operator=(const Signal&) = delete;
	Signal(Signal&&) = delete;
	Signal& operator=(Signal&&) = delete;

	[[nodiscard]] Connection Connect(DelegatePtr&& delegate) {
		const std::uint64_t id = _nextId++;
		_slots.push_back(Slot{id, std::move(delegate)});
		return Connection(this, id);
	}

	[[nodiscard]] Connection Connect(std::function<void(Args...)> func) {
		return Connect(std::make_unique<FunctionDelegate<Args...>>(std::move(func)));
	}

	template <class F>
	    requires std::is_invocable_v<F, Args...>
	[[nodiscard]] Connection Connect(F&& callable) {
		return Connect(std::function<void(Args...)>(std::forward<F>(callable)));
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
