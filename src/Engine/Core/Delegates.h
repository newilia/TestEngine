#pragma once

#include <functional>
#include <memory>
#include <utility>

template <class... TArgs>
class IDelegate
{
public:
	virtual ~IDelegate() {}

	virtual void operator()(TArgs... args) = 0;

	virtual bool expired() {
		return false;
	}
};

template <class TOwner, class... TArgs>
class IWeakDelegate : public IDelegate<TArgs...>
{
public:
	bool expired() override {
		return _owner.expired();
	}

protected:
	std::weak_ptr<TOwner> _owner;
};

template <class... TArgs>
class FunctionDelegate : public IDelegate<TArgs...>
{
public:
	FunctionDelegate(std::function<void(TArgs...)> func) {
		_function = std::move(func);
	}

	void operator()(TArgs... args) override {
		_function(args...);
	}

private:
	std::function<void(TArgs...)> _function;
};

template <class TOwner, class... TArgs>
class WeakMethodDelegate : public IWeakDelegate<TOwner, TArgs...>
{
public:
	WeakMethodDelegate(std::weak_ptr<TOwner> owner, void (TOwner::*method)(TArgs...)) {
		IWeakDelegate<TOwner, TArgs...>::_owner = owner;
		_method = method;
	}

	void operator()(TArgs... args) override {
		if (auto sharedOwner = IWeakDelegate<TOwner, TArgs...>::_owner.lock()) {
			(sharedOwner.get()->*_method)(args...);
		}
	}

private:
	void (TOwner::*_method)(TArgs...);
};

template <class TOwner, class... TArgs>
class WeakFunctionDelegate : public IWeakDelegate<TOwner, TArgs...>
{
public:
	WeakFunctionDelegate(std::weak_ptr<TOwner> owner, std::function<void(TArgs...)> func) {
		IWeakDelegate<TOwner, TArgs...>::_owner = owner;
		_function = func;
	}

	void operator()(TArgs... args) override {
		if (!IWeakDelegate<TOwner, TArgs...>::_owner.expired()) {
			_function(args...);
		}
	}

private:
	std::function<void(TArgs...)> _function;
};

template <class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(std::function<void(TArgs...)> func) {
	return std::make_unique<FunctionDelegate<TArgs...>>(std::move(func));
}

template <class TOwner, class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(std::weak_ptr<TOwner> owner, void (TOwner::*method)(TArgs...)) {
	return std::make_unique<WeakMethodDelegate<TOwner, TArgs...>>(owner, method);
}

template <class TOwner, class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(std::weak_ptr<TOwner> owner, std::function<void(TArgs...)> func) {
	auto p = new WeakFunctionDelegate<TOwner, TArgs...>(owner, func);
	return std::unique_ptr<IDelegate<TArgs...>>(p);
}
