#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class EntityOnNode;

namespace Engine {

	enum class SceneRefFilterKind
	{
		SceneNode,
		Entity,
	};

	/// UI hints separate from value type; used by the editor drawer only.
	struct PropertyMeta
	{
		std::string displayName;
		std::string tooltip;
		std::string mixedValueMarker = "mixed";
		bool readOnly = false;
		bool hasMixedValues = false;

		std::optional<double> numericMin;
		std::optional<double> numericMax;
		std::optional<double> dragSpeed;

		bool stringMultiline = false;

		/// For `Sequence` / `Associative` size controls (optional).
		std::optional<std::size_t> minElementCount;
		std::optional<std::size_t> maxElementCount;

		/// When non-empty, scalar controls use a combo filled from this callback (inspector-only).
		std::function<std::vector<std::string>()> valuesProviderStdString;
		std::function<std::vector<std::int32_t>()> valuesProviderInt32;
		std::function<std::vector<std::int64_t>()> valuesProviderInt64;
		std::function<std::vector<float>()> valuesProviderFloat;
		std::function<std::vector<double>()> valuesProviderDouble;

		/// For `PropertyKind::SceneRef` (inspector / picker).
		SceneRefFilterKind sceneRefFilterKind = SceneRefFilterKind::SceneNode;
		bool (*sceneRefEntityIsAllowed)(const std::shared_ptr<EntityOnNode>&) = nullptr;

		/// For `PropertyKind::AssetRef` (XML `type` attribute and editor browse defaults).
		std::string assetTypeId;
	};
} // namespace Engine
