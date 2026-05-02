#pragma once

#include <cassert>

#define Verify(...)                                                                                                    \
	([&] {                                                                                                             \
		auto _teVerifyV = (__VA_ARGS__);                                                                               \
		assert(static_cast<bool>(_teVerifyV));                                                                         \
		return _teVerifyV;                                                                                             \
	}())

#define VerifyFalse(...)                                                                                               \
	([&] {                                                                                                             \
		auto _teVerifyV = (__VA_ARGS__);                                                                               \
		assert(!static_cast<bool>(_teVerifyV));                                                                        \
		return _teVerifyV;                                                                                             \
	}())
