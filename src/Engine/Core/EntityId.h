#pragma once

#include <cstdint>

namespace Engine {

	/// Persistent handle for `SceneNode` / `EntityOnNode` within a scene. **0** means "null" for `RefWrapper`.
	using EntityId = std::uint32_t;

	inline constexpr EntityId kInvalidEntityId = 0;

} // namespace Engine
