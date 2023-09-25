#pragma once

template<class... TArgs>
class IDelegate {
public:
	virtual ~IDelegate() {}
	virtual void operator() (TArgs... args) = 0;
	virtual bool expired() { return false; }
};

template<class TOwner, class... TArgs>
class IWeakDelegate : public IDelegate<TArgs...> {
public:
	bool expired() override {
		return mOwner.expired();
	}
protected:
	std::weak_ptr<TOwner> mOwner;
};

template<class... TArgs>
class FunctionDelegate : public IDelegate<TArgs...> {
public:
	FunctionDelegate(std::function<void(TArgs...)> func) {
		mFunction = std::move(func);
	}
	void operator() (TArgs... args) override {
		mFunction(args...);
	}
private:
	std::function<void(TArgs...)> mFunction;
};

template<class TOwner, class... TArgs>
class WeakMethodDelegate : public IWeakDelegate<TOwner, TArgs...> {
public:
	WeakMethodDelegate(std::weak_ptr<TOwner> owner, void(TOwner::* method)(TArgs...)) {
		IWeakDelegate<TOwner, TArgs...>::mOwner = owner;
		mMethod = method;
	}
	void operator() (TArgs... args) override {
		if (auto sharedOwner = IWeakDelegate<TOwner, TArgs...>::mOwner.lock()) {
			(sharedOwner.get()->*mMethod)(args...);
		}
	}
private:
	void(TOwner::* mMethod)(TArgs...);
};

template<class TOwner, class... TArgs>
class WeakFunctionDelegate : public IWeakDelegate<TOwner, TArgs...> {
public:
	WeakFunctionDelegate(std::weak_ptr<TOwner> owner, std::function<void(TArgs...)> func) {
		IWeakDelegate<TOwner, TArgs...>::mOwner = owner;
		mFunction = func;
	}
	void operator() (TArgs... args) override {
		if (!IWeakDelegate<TOwner, TArgs...>::mOwner.expired()) {
			mFunction(args...);
		}
	}
private:
	std::function<void(TArgs...)> mFunction;
};



template <class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(auto func) {
	return std::make_unique<FunctionDelegate<TArgs...>>(func);
}

template <class TOwner, class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(std::weak_ptr<TOwner> owner, void(TOwner::* method)(TArgs...)) {
	return std::make_unique<WeakMethodDelegate<TOwner, TArgs...>>(owner, method);
}

template <class TOwner, class... TArgs>
std::unique_ptr<IDelegate<TArgs...>> createDelegate(std::weak_ptr<TOwner> owner, std::function<void(TArgs...)> func) {
	auto p = new WeakFunctionDelegate<TOwner, TArgs...>(owner, func);
	return std::unique_ptr<IDelegate<TArgs...>>(p);
}