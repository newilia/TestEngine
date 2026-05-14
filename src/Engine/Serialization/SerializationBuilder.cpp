#include "Engine/Serialization/SerializationBuilder.h"

#include <charconv>
#include <cstdint>
#include <limits>
#include <string>

namespace Engine::Serialization {
	namespace {

		constexpr const char kValueAttrName[] = "value";
		constexpr const char kXAttrName[] = "x";
		constexpr const char kYAttrName[] = "y";
		constexpr const char kZAttrName[] = "z";
		constexpr const char kRAttrName[] = "r";
		constexpr const char kGAttrName[] = "g";
		constexpr const char kBAttrName[] = "b";
		constexpr const char kAAttrName[] = "a";

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

	} // namespace

	SerializationBuilder::SerializationBuilder(pugi::xml_node node, Mode mode) : _node(node), _mode(mode) {}

	bool SerializationBuilder::IsSaving() const {
		return _mode == Mode::Save;
	}

	bool SerializationBuilder::IsLoading() const {
		return _mode == Mode::Load;
	}

	pugi::xml_node SerializationBuilder::GetNode() const {
		return _node;
	}

	SerializationBuilder SerializationBuilder::OpenObject(std::string_view objectName) {
		if (IsSaving()) {
			return SerializationBuilder(_node.append_child(objectName.data()), Mode::Save);
		}
		return SerializationBuilder(_node.child(objectName.data()), Mode::Load);
	}

	pugi::xml_node SerializationBuilder::RequireValueNode(std::string_view name, SerializationResult& result) {
		if (IsSaving()) {
			return _node.append_child(name.data());
		}
		const pugi::xml_node child = _node.child(name.data());
		if (!child) {
			result.AddError(std::string(name), "Missing field node");
		}
		return child;
	}

	void SerializationBuilder::Value(std::string_view name, bool& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kValueAttrName).set_value(value);
			return;
		}
		const pugi::xml_attribute attr = field.attribute(kValueAttrName);
		if (!attr) {
			result.AddError(std::string(name), "Missing bool value attribute");
			return;
		}
		value = attr.as_bool();
	}

	void SerializationBuilder::Value(std::string_view name, std::int32_t& value, SerializationResult& result) {
		std::int64_t parsed = value;
		Value(name, parsed, result);
		if (parsed < std::numeric_limits<std::int32_t>::min() || parsed > std::numeric_limits<std::int32_t>::max()) {
			result.AddError(std::string(name), "Int32 value out of range");
			return;
		}
		value = static_cast<std::int32_t>(parsed);
	}

	void SerializationBuilder::Value(std::string_view name, std::int64_t& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kValueAttrName).set_value(value);
			return;
		}
		const pugi::xml_attribute attr = field.attribute(kValueAttrName);
		if (!attr) {
			result.AddError(std::string(name), "Missing int64 value attribute");
			return;
		}
		std::string text = attr.value();
		if (!ParseInt(text, value)) {
			result.AddError(std::string(name), "Failed to parse int64 value");
		}
	}

	void SerializationBuilder::Value(std::string_view name, float& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kValueAttrName).set_value(value);
			return;
		}
		const pugi::xml_attribute attr = field.attribute(kValueAttrName);
		if (!attr) {
			result.AddError(std::string(name), "Missing float value attribute");
			return;
		}
		value = attr.as_float();
	}

	void SerializationBuilder::Value(std::string_view name, double& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kValueAttrName).set_value(value);
			return;
		}
		const pugi::xml_attribute attr = field.attribute(kValueAttrName);
		if (!attr) {
			result.AddError(std::string(name), "Missing double value attribute");
			return;
		}
		value = attr.as_double();
	}

	void SerializationBuilder::Value(std::string_view name, std::string& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kValueAttrName).set_value(value.c_str());
			return;
		}
		const pugi::xml_attribute attr = field.attribute(kValueAttrName);
		if (!attr) {
			result.AddError(std::string(name), "Missing string value attribute");
			return;
		}
		value = attr.as_string();
	}

	void SerializationBuilder::Value(std::string_view name, sf::Vector2f& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kXAttrName).set_value(value.x);
			field.append_attribute(kYAttrName).set_value(value.y);
			return;
		}
		const pugi::xml_attribute xAttr = field.attribute(kXAttrName);
		const pugi::xml_attribute yAttr = field.attribute(kYAttrName);
		if (!xAttr || !yAttr) {
			result.AddError(std::string(name), "Missing Vector2f attributes");
			return;
		}
		value.x = xAttr.as_float();
		value.y = yAttr.as_float();
	}

	void SerializationBuilder::Value(std::string_view name, sf::Vector2i& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kXAttrName).set_value(value.x);
			field.append_attribute(kYAttrName).set_value(value.y);
			return;
		}
		const pugi::xml_attribute xAttr = field.attribute(kXAttrName);
		const pugi::xml_attribute yAttr = field.attribute(kYAttrName);
		if (!xAttr || !yAttr) {
			result.AddError(std::string(name), "Missing Vector2i attributes");
			return;
		}
		value.x = xAttr.as_int();
		value.y = yAttr.as_int();
	}

	void SerializationBuilder::Value(std::string_view name, sf::Vector2u& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kXAttrName).set_value(value.x);
			field.append_attribute(kYAttrName).set_value(value.y);
			return;
		}
		const pugi::xml_attribute xAttr = field.attribute(kXAttrName);
		const pugi::xml_attribute yAttr = field.attribute(kYAttrName);
		if (!xAttr || !yAttr) {
			result.AddError(std::string(name), "Missing Vector2u attributes");
			return;
		}
		value.x = xAttr.as_uint();
		value.y = yAttr.as_uint();
	}

	void SerializationBuilder::Value(std::string_view name, sf::Vector3f& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kXAttrName).set_value(value.x);
			field.append_attribute(kYAttrName).set_value(value.y);
			field.append_attribute(kZAttrName).set_value(value.z);
			return;
		}
		const pugi::xml_attribute xAttr = field.attribute(kXAttrName);
		const pugi::xml_attribute yAttr = field.attribute(kYAttrName);
		const pugi::xml_attribute zAttr = field.attribute(kZAttrName);
		if (!xAttr || !yAttr || !zAttr) {
			result.AddError(std::string(name), "Missing Vector3f attributes");
			return;
		}
		value.x = xAttr.as_float();
		value.y = yAttr.as_float();
		value.z = zAttr.as_float();
	}

	void SerializationBuilder::Value(std::string_view name, sf::Color& value, SerializationResult& result) {
		pugi::xml_node field = RequireValueNode(name, result);
		if (!field) {
			return;
		}
		if (IsSaving()) {
			field.append_attribute(kRAttrName).set_value(value.r);
			field.append_attribute(kGAttrName).set_value(value.g);
			field.append_attribute(kBAttrName).set_value(value.b);
			field.append_attribute(kAAttrName).set_value(value.a);
			return;
		}
		const pugi::xml_attribute rAttr = field.attribute(kRAttrName);
		const pugi::xml_attribute gAttr = field.attribute(kGAttrName);
		const pugi::xml_attribute bAttr = field.attribute(kBAttrName);
		const pugi::xml_attribute aAttr = field.attribute(kAAttrName);
		if (!rAttr || !gAttr || !bAttr || !aAttr) {
			result.AddError(std::string(name), "Missing color attributes");
			return;
		}
		value = sf::Color(static_cast<std::uint8_t>(rAttr.as_uint()), static_cast<std::uint8_t>(gAttr.as_uint()),
		    static_cast<std::uint8_t>(bAttr.as_uint()), static_cast<std::uint8_t>(aAttr.as_uint()));
	}

} // namespace Engine::Serialization
