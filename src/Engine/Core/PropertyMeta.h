#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace Engine {
	/// UI hints separate from value type; used by the editor drawer only.
	struct PropertyMeta
	{
		std::string displayName;
		std::string tooltip;
		bool readOnly = false;

		std::optional<double> numericMin;
		std::optional<double> numericMax;
		std::optional<double> numericStep;
		std::optional<double> dragSpeed;

		bool stringMultiline = false;

		/// For `Sequence` / `Associative` size controls (optional).
		std::optional<std::size_t> minElementCount;
		std::optional<std::size_t> maxElementCount;
	};
} // namespace Engine
