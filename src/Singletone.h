#pragma once

template <typename T>
class Singletone {
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

private:
	inline static T* mInstance = nullptr;
};