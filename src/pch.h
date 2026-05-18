#pragma once

// Also force-included on every TU via CMake (/FI or -include); listed here so the PCH unit matches.
#include "TestEngineGlobals.h"

// C++ standard library
#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

// SFML (largest third-party compile cost in this target)
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

// fmt / Dear ImGui (editor and diagnostics)
#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>
