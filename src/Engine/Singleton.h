#pragma once

template <typename T>
class Singleton
{
public:
	static T* getInstance() {
		if (!_instance) {
			_instance = new T();
		}
		return _instance;
	}

	static void destroy() {
		delete _instance;
		_instance = nullptr;
	}

	static bool isAlive() { return _instance != nullptr; }

protected:
	Singleton() = default;
	virtual ~Singleton() = default;

private:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

	inline static T* _instance = nullptr;
};