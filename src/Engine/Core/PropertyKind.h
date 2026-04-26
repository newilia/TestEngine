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
		Vec3f,
		Color,
		/// Group / struct: only `children` are shown.
		Object,
		/// Indexed list: `children` are elements; optional resize via `sequence`.
		Sequence,
		/// Key–value collection: `children` are alternating or grouped pair nodes; optional add/remove.
		Associative,
	};

} // namespace Engine
