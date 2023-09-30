#pragma once

template <typename T>
class Singleton {
public:
	static T* getInstance() {
		if (!mInstance) {
			mInstance = new T();
		}
		return mInstance;
	}

	static void destroy() {
		delete mInstance;
		mInstance = nullptr;
	}

	static bool isAlive() {
		return mInstance != nullptr;
	}

protected:
	Singleton() = default;
	virtual ~Singleton() = default;

private:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	inline static T* mInstance = nullptr;
};