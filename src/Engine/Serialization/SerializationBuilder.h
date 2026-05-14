#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <pugixml.hpp>

#include <cstdint>
#include <string>
#include <string_view>

namespace Engine::Serialization {

	class SerializationBuilder
	{
	public:
		enum class Mode
		{
			Save,
			Load
		};

		SerializationBuilder(pugi::xml_node node, Mode mode);

		[[nodiscard]] bool IsSaving() const;
		[[nodiscard]] bool IsLoading() const;

		pugi::xml_node GetNode() const;

		SerializationBuilder OpenObject(std::string_view objectName);

		void Value(std::string_view name, bool& value, SerializationResult& result);
		void Value(std::string_view name, std::int32_t& value, SerializationResult& result);
		void Value(std::string_view name, std::int64_t& value, SerializationResult& result);
		void Value(std::string_view name, float& value, SerializationResult& result);
		void Value(std::string_view name, double& value, SerializationResult& result);
		void Value(std::string_view name, std::string& value, SerializationResult& result);
		void Value(std::string_view name, sf::Vector2f& value, SerializationResult& result);
		void Value(std::string_view name, sf::Vector2i& value, SerializationResult& result);
		void Value(std::string_view name, sf::Vector2u& value, SerializationResult& result);
		void Value(std::string_view name, sf::Vector3f& value, SerializationResult& result);
		void Value(std::string_view name, sf::Color& value, SerializationResult& result);

	private:
		pugi::xml_node RequireValueNode(std::string_view name, SerializationResult& result);
		pugi::xml_node _node;
		Mode _mode = Mode::Save;
	};

} // namespace Engine::Serialization
