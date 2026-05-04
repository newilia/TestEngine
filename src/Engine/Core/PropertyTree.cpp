#include "Engine/Core/PropertyTree.h"

namespace Engine::PropertyTreeDetail {
	std::string effectiveLabel(const std::string& label, const PropertyMeta& meta) {
		if (!meta.displayName.empty()) {
			return meta.displayName;
		}
		return label;
	}
} // namespace Engine::PropertyTreeDetail

namespace Engine {
	namespace {
		PropertyNode makeLeaf(std::string id, const std::string& label, PropertyKind kind, PropertyMeta meta,
		                      PropertyAccess access) {
			PropertyNode n;
			n.id = std::move(id);
			n.label = PropertyTreeDetail::effectiveLabel(label, meta);
			n.kind = kind;
			n.meta = std::move(meta);
			n.access = std::move(access);
			return n;
		}
	} // namespace

	PropertyBuilder::PropertyBuilder(PropertyTree& tree) : _tree(&tree) {}

	void PropertyBuilder::clear() {
		_tree->roots.clear();
		_tree->inspectorMethods.clear();
		_stack.clear();
	}

	PropertyNode& PropertyBuilder::appendChild(PropertyNode&& node) {
		if (_stack.empty()) {
			_tree->roots.push_back(std::move(node));
			return _tree->roots.back();
		}
		_stack.back()->children.push_back(std::move(node));
		return _stack.back()->children.back();
	}

	PropertyNode& PropertyBuilder::pushObject(std::string id, const std::string& label, PropertyMeta meta) {
		PropertyNode n;
		n.id = std::move(id);
		n.label = PropertyTreeDetail::effectiveLabel(label, meta);
		n.kind = PropertyKind::Object;
		n.meta = std::move(meta);
		n.access = PropAccessNone{};
		PropertyNode& ref = appendChild(std::move(n));
		_stack.push_back(&ref);
		return ref;
	}

	void PropertyBuilder::pop() {
		if (!_stack.empty()) {
			_stack.pop_back();
		}
	}

	void PropertyBuilder::addBool(std::string id, const std::string& label, std::function<bool()> get,
	                              std::function<void(bool)> set, PropertyMeta meta) {
		PropAccessBool acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Bool, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addInt32(std::string id, const std::string& label, std::function<std::int32_t()> get,
	                               std::function<void(std::int32_t)> set, PropertyMeta meta) {
		PropAccessInt32 acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Int32, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addInt64(std::string id, const std::string& label, std::function<std::int64_t()> get,
	                               std::function<void(std::int64_t)> set, PropertyMeta meta) {
		PropAccessInt64 acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Int64, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addFloat(std::string id, const std::string& label, std::function<float()> get,
	                               std::function<void(float)> set, PropertyMeta meta) {
		PropAccessFloat acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Float, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addDouble(std::string id, const std::string& label, std::function<double()> get,
	                                std::function<void(double)> set, PropertyMeta meta) {
		PropAccessDouble acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Double, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addString(std::string id, const std::string& label, std::function<std::string()> get,
	                                std::function<void(std::string)> set, PropertyMeta meta) {
		PropAccessString acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::String, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addEnum(std::string id, const std::string& label, std::function<int()> get,
	                              std::function<void(int)> set, std::vector<std::pair<int, std::string>> options,
	                              PropertyMeta meta) {
		PropAccessEnum acc{std::move(get), std::move(set), std::move(options)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Enum, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addVec2f(std::string id, const std::string& label, std::function<sf::Vector2f()> get,
	                               std::function<void(sf::Vector2f)> set, PropertyMeta meta) {
		PropAccessVec2f acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Vec2f, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addVec2i(std::string id, const std::string& label, std::function<sf::Vector2i()> get,
	                               std::function<void(sf::Vector2i)> set, PropertyMeta meta) {
		PropAccessVec2i acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Vec2i, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addVec2u(std::string id, const std::string& label, std::function<sf::Vector2u()> get,
	                               std::function<void(sf::Vector2u)> set, PropertyMeta meta) {
		PropAccessVec2u acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Vec2u, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addVec3f(std::string id, const std::string& label, std::function<sf::Vector3f()> get,
	                               std::function<void(sf::Vector3f)> set, PropertyMeta meta) {
		PropAccessVec3f acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Vec3f, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::addColor(std::string id, const std::string& label, std::function<sf::Color()> get,
	                               std::function<void(sf::Color)> set, PropertyMeta meta) {
		PropAccessColor acc{std::move(get), std::move(set)};
		(void)appendChild(makeLeaf(std::move(id), label, PropertyKind::Color, std::move(meta), std::move(acc)));
	}

	void PropertyBuilder::beginSequence(std::string id, const std::string& label, PropAccessSequence sequenceOps,
	                                    PropertyMeta meta) {
		PropertyNode n;
		n.id = std::move(id);
		n.label = PropertyTreeDetail::effectiveLabel(label, meta);
		n.kind = PropertyKind::Sequence;
		n.meta = std::move(meta);
		n.access = std::move(sequenceOps);
		PropertyNode& ref = appendChild(std::move(n));
		_stack.push_back(&ref);
	}

	void PropertyBuilder::endSequence() {
		pop();
	}

	void PropertyBuilder::beginAssociative(std::string id, const std::string& label,
	                                       PropAccessAssociative associativeOps, PropertyMeta meta) {
		PropertyNode n;
		n.id = std::move(id);
		n.label = PropertyTreeDetail::effectiveLabel(label, meta);
		n.kind = PropertyKind::Associative;
		n.meta = std::move(meta);
		n.access = std::move(associativeOps);
		PropertyNode& ref = appendChild(std::move(n));
		_stack.push_back(&ref);
	}

	void PropertyBuilder::endAssociative() {
		pop();
	}

	void PropertyBuilder::registerInspectorMethod(std::string menuLabel, std::function<void()> invoke) {
		_tree->inspectorMethods.push_back(ReflectedInspectorMethod{std::move(menuLabel), std::move(invoke)});
	}
} // namespace Engine
