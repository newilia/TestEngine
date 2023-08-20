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

	static void cleanup() {
		delete mInstance;
		mInstance = nullptr;
	}

	static bool isAlive() {
		return mInstance != nullptr;
	}

protected:
	inline static T* mInstance = nullptr;
};