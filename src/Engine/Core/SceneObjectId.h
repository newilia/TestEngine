#pragma once

#include <cstdint>

namespace Engine {

	/// Persistent handle for `SceneNode` / `EntityOnNode` within a scene. **0** means "null" for `RefWrapper`.
	using SceneObjectId = std::uint32_t;

	inline constexpr SceneObjectId kInvalidSceneObjectId = 0;

} // namespace Engine
