#pragma once

namespace Engine {

	enum class PropertyKind
	{
		Bool,
		Int32,
		Int64,
		Float,
		Double,
		String,
		Enum,
		Vec2f,
		Vec2i,
		Vec2u,
		Vec3f,
		Color,
		/// Serialized scene object id (`RefWrapper`); editor uses hierarchy picker.
		SceneRef,
		/// Serialized content-relative asset path (`AssetRef<T>`); editor uses path combo / browse.
		AssetRef,
		/// Group / struct: only `children` are shown.
		Object,
		/// Indexed list: `children` are elements; optional resize via `sequence`.
		Sequence,
		/// Key–value collection: `children` are alternating or grouped pair nodes; optional add/remove.
		Associative,
	};

} // namespace Engine
