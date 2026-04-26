#pragma once

#include "Engine/Core/PropertyKind.h"
#include "Engine/Core/PropertyMeta.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Engine {

	struct PropAccessNone
	{};

	struct PropAccessBool
	{
		std::function<bool()> get;
		std::function<void(bool)> set;
	};

	struct PropAccessInt32
	{
		std::function<std::int32_t()> get;
		std::function<void(std::int32_t)> set;
	};

	struct PropAccessInt64
	{
		std::function<std::int64_t()> get;
		std::function<void(std::int64_t)> set;
	};

	struct PropAccessFloat
	{
		std::function<float()> get;
		std::function<void(float)> set;
	};

	struct PropAccessDouble
	{
		std::function<double()> get;
		std::function<void(double)> set;
	};

	struct PropAccessString
	{
		std::function<std::string()> get;
		std::function<void(std::string)> set;
	};

	struct PropAccessEnum
	{
		std::function<int()> get;
		std::function<void(int)> set;
		std::vector<std::pair<int, std::string>> options;
	};

	struct PropAccessVec2f
	{
		std::function<sf::Vector2f()> get;
		std::function<void(sf::Vector2f)> set;
	};

	struct PropAccessVec3f
	{
		std::function<sf::Vector3f()> get;
		std::function<void(sf::Vector3f)> set;
	};

	struct PropAccessColor
	{
		std::function<sf::Color()> get;
		std::function<void(sf::Color)> set;
	};

	/// Optional size control for `PropertyKind::Sequence` lists.
	struct PropAccessSequence
	{
		std::function<std::size_t()> getSize;
		std::function<void(std::size_t)> resize;
	};

	/// Optional add/remove for `PropertyKind::Associative` maps / sets.
	struct PropAccessAssociative
	{
		std::function<void()> addPair;
		std::function<void(std::size_t pairIndex)> removePair;
	};

	using PropertyAccess =
	    std::variant<PropAccessNone, PropAccessBool, PropAccessInt32, PropAccessInt64, PropAccessFloat,
	                 PropAccessDouble, PropAccessString, PropAccessEnum, PropAccessVec2f, PropAccessVec3f,
	                 PropAccessColor, PropAccessSequence, PropAccessAssociative>;

	struct PropertyNode
	{
		std::string id;
		std::string label;
		PropertyKind kind = PropertyKind::Object;
		PropertyMeta meta;
		PropertyAccess access;
		std::vector<PropertyNode> children;
	};

} // namespace Engine
