#pragma once

#include "Engine/Core/PropertyNode.h"

#include <string>
#include <utility>
#include <vector>

namespace Engine {

	struct PropertyTree
	{
		std::vector<PropertyNode> roots;
	};

	/// Stack-based helper used from `IInspectable::BuildPropertyTree`.
	class PropertyBuilder
	{
	public:
		explicit PropertyBuilder(PropertyTree& tree);

		void clear();

		PropertyNode& pushObject(std::string id, const std::string& label, PropertyMeta meta = {});
		void pop();

		void addBool(std::string id, const std::string& label, std::function<bool()> get, std::function<void(bool)> set,
		             PropertyMeta meta = {});

		void addInt32(std::string id, const std::string& label, std::function<std::int32_t()> get,
		              std::function<void(std::int32_t)> set, PropertyMeta meta = {});

		void addInt64(std::string id, const std::string& label, std::function<std::int64_t()> get,
		              std::function<void(std::int64_t)> set, PropertyMeta meta = {});

		void addFloat(std::string id, const std::string& label, std::function<float()> get,
		              std::function<void(float)> set, PropertyMeta meta = {});

		void addDouble(std::string id, const std::string& label, std::function<double()> get,
		               std::function<void(double)> set, PropertyMeta meta = {});

		void addString(std::string id, const std::string& label, std::function<std::string()> get,
		               std::function<void(std::string)> set, PropertyMeta meta = {});

		void addEnum(std::string id, const std::string& label, std::function<int()> get, std::function<void(int)> set,
		             std::vector<std::pair<int, std::string>> options, PropertyMeta meta = {});

		void addVec2f(std::string id, const std::string& label, std::function<sf::Vector2f()> get,
		              std::function<void(sf::Vector2f)> set, PropertyMeta meta = {});

		void addVec3f(std::string id, const std::string& label, std::function<sf::Vector3f()> get,
		              std::function<void(sf::Vector3f)> set, PropertyMeta meta = {});

		void addColor(std::string id, const std::string& label, std::function<sf::Color()> get,
		              std::function<void(sf::Color)> set, PropertyMeta meta = {});
		/// Begins a sequence node; push children (e.g. one object per element), then `endSequence()`.
		void beginSequence(std::string id, const std::string& label, PropAccessSequence sequenceOps,
		                   PropertyMeta meta = {});

		void endSequence();

		/// Begins an associative node (map/set-like); children are built by the caller (e.g. pair objects).
		void beginAssociative(std::string id, const std::string& label, PropAccessAssociative associativeOps,
		                      PropertyMeta meta = {});

		void endAssociative();

	private:
		PropertyTree* _tree = nullptr;
		std::vector<PropertyNode*> _stack;

		PropertyNode& appendChild(PropertyNode&& node);
	};

} // namespace Engine
