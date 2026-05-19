#pragma once

#include <cassert>

#if !defined(NDEBUG)
#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include <fmt/format.h>

namespace detail {

	inline void LogDebugOutput(std::string_view message) {
#ifdef _WIN32
		std::string line(message);
		if (line.empty() || line.back() != '\n')
			line.push_back('\n');
		::OutputDebugStringA(line.c_str());
#else
		fmt::print(stderr, "{}\n", message);
#endif
	}

} // namespace detail

#define LOG_DEBUG(...)                                                                                                 \
	do {                                                                                                               \
		::detail::LogDebugOutput(fmt::format(__VA_ARGS__));                                                            \
	}                                                                                                                  \
	while (0)

#else

#define LOG_DEBUG(...) ((void)0)

#endif

// TODO #ifdef DEBUG
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
