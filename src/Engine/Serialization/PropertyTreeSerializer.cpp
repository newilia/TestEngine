#include "Engine/Serialization/PropertyTreeSerializer.h"

#include "Engine/Core/PropertyNode.h"
#include "Engine/Core/PropertyTree.h"

#include <charconv>
#include <cstdint>
#include <string>
#include <vector>

namespace Engine::Serialization {
	namespace {

		constexpr const char kPropertyElement[] = "Property";
		constexpr const char kIdAttr[] = "id";
		constexpr const char kKindAttr[] = "kind";
		constexpr const char kValueAttr[] = "value";
		constexpr const char kTypeAttr[] = "type";
		constexpr const char kNameAttr[] = "name";
		constexpr const char kXAttr[] = "x";
		constexpr const char kYAttr[] = "y";
		constexpr const char kZAttr[] = "z";
		constexpr const char kRAttr[] = "r";
		constexpr const char kGAttr[] = "g";
		constexpr const char kBAttr[] = "b";
		constexpr const char kAAttr[] = "a";

		std::string PropertyKindToString(PropertyKind kind) {
			switch (kind) {
			case PropertyKind::Bool:
				return "Bool";
			case PropertyKind::Int32:
				return "Int32";
			case PropertyKind::Int64:
				return "Int64";
			case PropertyKind::Float:
				return "Float";
			case PropertyKind::Double:
				return "Double";
			case PropertyKind::String:
				return "String";
			case PropertyKind::Enum:
				return "Enum";
			case PropertyKind::Vec2f:
				return "Vec2f";
			case PropertyKind::Vec2i:
				return "Vec2i";
			case PropertyKind::Vec2u:
				return "Vec2u";
			case PropertyKind::Vec3f:
				return "Vec3f";
			case PropertyKind::Color:
				return "Color";
			case PropertyKind::SceneRef:
				return "SceneRef";
			case PropertyKind::AssetRef:
				return "AssetRef";
			case PropertyKind::Object:
				return "Object";
			case PropertyKind::Sequence:
				return "Sequence";
			case PropertyKind::Associative:
				return "Associative";
			}
			return "Unknown";
		}

		std::string AppendPath(const std::string& base, const std::string& segment) {
			if (base.empty()) {
				return segment;
			}
			return base + "." + segment;
		}

		pugi::xml_node FindChildPropertyById(const pugi::xml_node& parent, const std::string& id) {
			for (pugi::xml_node child : parent.children(kPropertyElement)) {
				if (std::string(child.attribute(kIdAttr).as_string()) == id) {
					return child;
				}
			}
			return {};
		}

		std::size_t CountXmlPropertyChildren(const pugi::xml_node& parent) {
			std::size_t count = 0;
			for (const pugi::xml_node child : parent.children(kPropertyElement)) {
				(void)child;
				++count;
			}
			return count;
		}

		std::vector<pugi::xml_node> GatherXmlChildren(const pugi::xml_node& parent) {
			std::vector<pugi::xml_node> result;
			for (pugi::xml_node child : parent.children(kPropertyElement)) {
				result.push_back(child);
			}
			return result;
		}

		template <typename TInt>
		bool ParseInt(const std::string& text, TInt& outValue) {
			const char* begin = text.data();
			const char* end = text.data() + text.size();
			TInt parsed{};
			const auto [ptr, ec] = std::from_chars(begin, end, parsed);
			if (ec != std::errc{} || ptr != end) {
				return false;
			}
			outValue = parsed;
			return true;
		}

		void SavePropertyNode(const PropertyNode& node, pugi::xml_node parentNode, SerializationResult& result) {
			pugi::xml_node xmlProperty = parentNode.append_child(kPropertyElement);
			xmlProperty.append_attribute(kIdAttr).set_value(node.id.c_str());
			xmlProperty.append_attribute(kKindAttr).set_value(PropertyKindToString(node.kind).c_str());

			switch (node.kind) {
			case PropertyKind::Bool: {
				const auto* access = std::get_if<PropAccessBool>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing bool getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::Int32: {
				const auto* access = std::get_if<PropAccessInt32>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing int32 getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::Int64: {
				const auto* access = std::get_if<PropAccessInt64>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing int64 getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::Float: {
				const auto* access = std::get_if<PropAccessFloat>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing float getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::Double: {
				const auto* access = std::get_if<PropAccessDouble>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing double getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::String: {
				const auto* access = std::get_if<PropAccessString>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing string getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get().c_str());
				return;
			}
			case PropertyKind::Enum: {
				const auto* access = std::get_if<PropAccessEnum>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing enum getter");
					return;
				}
				const int value = access->get();
				xmlProperty.append_attribute(kValueAttr).set_value(value);
				for (const auto& [optionValue, optionName] : access->options) {
					if (optionValue == value) {
						xmlProperty.append_attribute(kNameAttr).set_value(optionName.c_str());
						break;
					}
				}
				return;
			}
			case PropertyKind::Vec2f: {
				const auto* access = std::get_if<PropAccessVec2f>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing vec2f getter");
					return;
				}
				const sf::Vector2f value = access->get();
				xmlProperty.append_attribute(kXAttr).set_value(value.x);
				xmlProperty.append_attribute(kYAttr).set_value(value.y);
				return;
			}
			case PropertyKind::Vec2i: {
				const auto* access = std::get_if<PropAccessVec2i>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing vec2i getter");
					return;
				}
				const sf::Vector2i value = access->get();
				xmlProperty.append_attribute(kXAttr).set_value(value.x);
				xmlProperty.append_attribute(kYAttr).set_value(value.y);
				return;
			}
			case PropertyKind::Vec2u: {
				const auto* access = std::get_if<PropAccessVec2u>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing vec2u getter");
					return;
				}
				const sf::Vector2u value = access->get();
				xmlProperty.append_attribute(kXAttr).set_value(value.x);
				xmlProperty.append_attribute(kYAttr).set_value(value.y);
				return;
			}
			case PropertyKind::Vec3f: {
				const auto* access = std::get_if<PropAccessVec3f>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing vec3f getter");
					return;
				}
				const sf::Vector3f value = access->get();
				xmlProperty.append_attribute(kXAttr).set_value(value.x);
				xmlProperty.append_attribute(kYAttr).set_value(value.y);
				xmlProperty.append_attribute(kZAttr).set_value(value.z);
				return;
			}
			case PropertyKind::Color: {
				const auto* access = std::get_if<PropAccessColor>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing color getter");
					return;
				}
				const sf::Color value = access->get();
				xmlProperty.append_attribute(kRAttr).set_value(value.r);
				xmlProperty.append_attribute(kGAttr).set_value(value.g);
				xmlProperty.append_attribute(kBAttr).set_value(value.b);
				xmlProperty.append_attribute(kAAttr).set_value(value.a);
				return;
			}
			case PropertyKind::SceneRef: {
				const auto* access = std::get_if<PropAccessSceneRef>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing SceneRef getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get());
				return;
			}
			case PropertyKind::AssetRef: {
				const auto* access = std::get_if<PropAccessAssetRef>(&node.access);
				if (!access || !access->get) {
					result.AddError(node.id, "Missing AssetRef getter");
					return;
				}
				xmlProperty.append_attribute(kValueAttr).set_value(access->get().c_str());
				if (!node.meta.assetTypeId.empty()) {
					xmlProperty.append_attribute(kTypeAttr).set_value(node.meta.assetTypeId.c_str());
				}
				return;
			}
			case PropertyKind::Object:
			case PropertyKind::Sequence:
			case PropertyKind::Associative: {
				if (node.kind == PropertyKind::Object) {
					if (const auto* typeAccess = std::get_if<PropAccessMetaClassTypeId>(&node.access)) {
						if (typeAccess->getTypeId) {
							const std::string typeId = typeAccess->getTypeId();
							if (!typeId.empty()) {
								xmlProperty.append_attribute(kTypeAttr).set_value(typeId.c_str());
							}
						}
					}
				}
				for (const PropertyNode& child : node.children) {
					SavePropertyNode(child, xmlProperty, result);
				}
				return;
			}
			}
		}

		void AdjustContainersByXml(const pugi::xml_node& xmlNode, PropertyNode& targetNode) {
			if (targetNode.kind == PropertyKind::Sequence) {
				if (auto* polyAccess = std::get_if<PropAccessPolymorphicSequence>(&targetNode.access)) {
					const std::vector<pugi::xml_node> xmlChildren = GatherXmlChildren(xmlNode);
					std::size_t runtimeSize = polyAccess->getSize ? polyAccess->getSize() : targetNode.children.size();
					while (runtimeSize > xmlChildren.size() && polyAccess->remove) {
						polyAccess->remove(runtimeSize - 1);
						--runtimeSize;
					}
					for (std::size_t i = runtimeSize; i < xmlChildren.size(); ++i) {
						const pugi::xml_attribute typeAttr = xmlChildren[i].attribute(kTypeAttr);
						if (polyAccess->emplace && typeAttr) {
							polyAccess->emplace(typeAttr.as_string());
						}
					}
					runtimeSize = polyAccess->getSize ? polyAccess->getSize() : xmlChildren.size();
					for (std::size_t i = 0; i < xmlChildren.size() && i < runtimeSize; ++i) {
						const pugi::xml_attribute typeAttr = xmlChildren[i].attribute(kTypeAttr);
						if (typeAttr && polyAccess->replaceAt) {
							polyAccess->replaceAt(i, typeAttr.as_string());
						}
					}
					return;
				}
				if (auto* access = std::get_if<PropAccessSequence>(&targetNode.access)) {
					if (access->resize) {
						access->resize(CountXmlPropertyChildren(xmlNode));
					}
				}
				return;
			}
			if (targetNode.kind == PropertyKind::Associative) {
				if (auto* access = std::get_if<PropAccessAssociative>(&targetNode.access)) {
					std::size_t targetCount = targetNode.children.size();
					const std::size_t xmlCount = CountXmlPropertyChildren(xmlNode);
					while (targetCount < xmlCount && access->addPair) {
						access->addPair();
						++targetCount;
					}
					while (targetCount > xmlCount && access->removePair) {
						access->removePair(targetCount - 1);
						--targetCount;
					}
				}
				return;
			}
			for (PropertyNode& child : targetNode.children) {
				const pugi::xml_node xmlChild = FindChildPropertyById(xmlNode, child.id);
				if (xmlChild) {
					AdjustContainersByXml(xmlChild, child);
				}
			}
		}

		void LoadLeafValue(const pugi::xml_node& xmlNode, PropertyNode& targetNode, const std::string& path,
		    SerializationResult& result) {
			switch (targetNode.kind) {
			case PropertyKind::Bool: {
				auto* access = std::get_if<PropAccessBool>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing bool setter or value");
					return;
				}
				access->set(valueAttr.as_bool());
				return;
			}
			case PropertyKind::Int32: {
				auto* access = std::get_if<PropAccessInt32>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing int32 setter or value");
					return;
				}
				access->set(valueAttr.as_int());
				return;
			}
			case PropertyKind::Int64: {
				auto* access = std::get_if<PropAccessInt64>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing int64 setter or value");
					return;
				}
				std::int64_t parsed = 0;
				if (!ParseInt(std::string(valueAttr.as_string()), parsed)) {
					result.AddError(path, "Failed to parse int64");
					return;
				}
				access->set(parsed);
				return;
			}
			case PropertyKind::Float: {
				auto* access = std::get_if<PropAccessFloat>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing float setter or value");
					return;
				}
				access->set(valueAttr.as_float());
				return;
			}
			case PropertyKind::Double: {
				auto* access = std::get_if<PropAccessDouble>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing double setter or value");
					return;
				}
				access->set(valueAttr.as_double());
				return;
			}
			case PropertyKind::String: {
				auto* access = std::get_if<PropAccessString>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing string setter or value");
					return;
				}
				access->set(valueAttr.as_string());
				return;
			}
			case PropertyKind::Enum: {
				auto* access = std::get_if<PropAccessEnum>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing enum setter");
					return;
				}
				const std::string optionName = xmlNode.attribute(kNameAttr).as_string();
				if (!optionName.empty()) {
					for (const auto& [value, name] : access->options) {
						if (name == optionName) {
							access->set(value);
							return;
						}
					}
				}
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!valueAttr) {
					result.AddError(path, "Missing enum value");
					return;
				}
				access->set(valueAttr.as_int());
				return;
			}
			case PropertyKind::Vec2f: {
				auto* access = std::get_if<PropAccessVec2f>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing vec2f setter");
					return;
				}
				const pugi::xml_attribute xAttr = xmlNode.attribute(kXAttr);
				const pugi::xml_attribute yAttr = xmlNode.attribute(kYAttr);
				if (!xAttr || !yAttr) {
					result.AddError(path, "Missing vec2f coordinates");
					return;
				}
				access->set(sf::Vector2f(xAttr.as_float(), yAttr.as_float()));
				return;
			}
			case PropertyKind::Vec2i: {
				auto* access = std::get_if<PropAccessVec2i>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing vec2i setter");
					return;
				}
				const pugi::xml_attribute xAttr = xmlNode.attribute(kXAttr);
				const pugi::xml_attribute yAttr = xmlNode.attribute(kYAttr);
				if (!xAttr || !yAttr) {
					result.AddError(path, "Missing vec2i coordinates");
					return;
				}
				access->set(sf::Vector2i(xAttr.as_int(), yAttr.as_int()));
				return;
			}
			case PropertyKind::Vec2u: {
				auto* access = std::get_if<PropAccessVec2u>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing vec2u setter");
					return;
				}
				const pugi::xml_attribute xAttr = xmlNode.attribute(kXAttr);
				const pugi::xml_attribute yAttr = xmlNode.attribute(kYAttr);
				if (!xAttr || !yAttr) {
					result.AddError(path, "Missing vec2u coordinates");
					return;
				}
				access->set(sf::Vector2u(xAttr.as_uint(), yAttr.as_uint()));
				return;
			}
			case PropertyKind::Vec3f: {
				auto* access = std::get_if<PropAccessVec3f>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing vec3f setter");
					return;
				}
				const pugi::xml_attribute xAttr = xmlNode.attribute(kXAttr);
				const pugi::xml_attribute yAttr = xmlNode.attribute(kYAttr);
				const pugi::xml_attribute zAttr = xmlNode.attribute(kZAttr);
				if (!xAttr || !yAttr || !zAttr) {
					result.AddError(path, "Missing vec3f coordinates");
					return;
				}
				access->set(sf::Vector3f(xAttr.as_float(), yAttr.as_float(), zAttr.as_float()));
				return;
			}
			case PropertyKind::Color: {
				auto* access = std::get_if<PropAccessColor>(&targetNode.access);
				if (!access || !access->set) {
					result.AddError(path, "Missing color setter");
					return;
				}
				const pugi::xml_attribute rAttr = xmlNode.attribute(kRAttr);
				const pugi::xml_attribute gAttr = xmlNode.attribute(kGAttr);
				const pugi::xml_attribute bAttr = xmlNode.attribute(kBAttr);
				const pugi::xml_attribute aAttr = xmlNode.attribute(kAAttr);
				if (!rAttr || !gAttr || !bAttr || !aAttr) {
					result.AddError(path, "Missing color channels");
					return;
				}
				access->set(
				    sf::Color(static_cast<std::uint8_t>(rAttr.as_uint()), static_cast<std::uint8_t>(gAttr.as_uint()),
				        static_cast<std::uint8_t>(bAttr.as_uint()), static_cast<std::uint8_t>(aAttr.as_uint())));
				return;
			}
			case PropertyKind::SceneRef: {
				auto* access = std::get_if<PropAccessSceneRef>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing SceneRef setter or value");
					return;
				}
				access->set(static_cast<std::uint32_t>(valueAttr.as_uint()));
				return;
			}
			case PropertyKind::AssetRef: {
				auto* access = std::get_if<PropAccessAssetRef>(&targetNode.access);
				const pugi::xml_attribute valueAttr = xmlNode.attribute(kValueAttr);
				if (!access || !access->set || !valueAttr) {
					result.AddError(path, "Missing AssetRef setter or value");
					return;
				}
				access->set(valueAttr.as_string());
				return;
			}
			case PropertyKind::Object:
			case PropertyKind::Sequence:
			case PropertyKind::Associative:
				return;
			}
		}

		void LoadPropertyNode(const pugi::xml_node& xmlNode, PropertyNode& targetNode, const std::string& path,
		    SerializationResult& result) {
			LoadLeafValue(xmlNode, targetNode, path, result);

			if (targetNode.kind == PropertyKind::Object || targetNode.kind == PropertyKind::Sequence ||
			    targetNode.kind == PropertyKind::Associative) {
				const std::vector<pugi::xml_node> xmlChildren = GatherXmlChildren(xmlNode);
				for (std::size_t i = 0; i < targetNode.children.size(); ++i) {
					PropertyNode& runtimeChild = targetNode.children[i];
					pugi::xml_node xmlChild = FindChildPropertyById(xmlNode, runtimeChild.id);
					if (!xmlChild && i < xmlChildren.size()) {
						xmlChild = xmlChildren[i];
					}
					if (!xmlChild) {
						continue;
					}
					LoadPropertyNode(xmlChild, runtimeChild, AppendPath(path, runtimeChild.id), result);
				}
			}
		}

		void BuildPropertyTreeForProvider(IPropertiesProvider& provider, PropertyTree& tree, PropertyBuilder& builder) {
			tree.roots.clear();
			tree.inspectorMethods.clear();
			builder.clear();
			provider.BuildPropertyTree(builder);
		}

	} // namespace

	SerializationResult PropertyTreeSerializer::SaveProvider(
	    IPropertiesProvider& provider, pugi::xml_node propertiesNode) {
		SerializationResult result;
		PropertyTree propertyTree;
		PropertyBuilder builder(propertyTree);
		BuildPropertyTreeForProvider(provider, propertyTree, builder);

		for (const PropertyNode& root : propertyTree.roots) {
			SavePropertyNode(root, propertiesNode, result);
		}
		return result;
	}

	SerializationResult PropertyTreeSerializer::LoadProvider(
	    IPropertiesProvider& provider, const pugi::xml_node& propertiesNode) {
		SerializationResult result;
		PropertyTree propertyTree;
		PropertyBuilder builder(propertyTree);
		BuildPropertyTreeForProvider(provider, propertyTree, builder);

		for (PropertyNode& root : propertyTree.roots) {
			const pugi::xml_node xmlRoot = FindChildPropertyById(propertiesNode, root.id);
			if (xmlRoot) {
				AdjustContainersByXml(xmlRoot, root);
			}
		}

		BuildPropertyTreeForProvider(provider, propertyTree, builder);

		for (PropertyNode& root : propertyTree.roots) {
			const pugi::xml_node xmlRoot = FindChildPropertyById(propertiesNode, root.id);
			if (!xmlRoot) {
				continue;
			}
			LoadPropertyNode(xmlRoot, root, root.id, result);
		}

		return result;
	}

} // namespace Engine::Serialization
