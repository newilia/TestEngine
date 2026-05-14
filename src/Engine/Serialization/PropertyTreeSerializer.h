#pragma once

#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Serialization/SerializationResult.h"

#include <pugixml.hpp>

namespace Engine::Serialization {

	class PropertyTreeSerializer
	{
	public:
		static SerializationResult SaveProvider(IPropertiesProvider& provider, pugi::xml_node propertiesNode);
		static SerializationResult LoadProvider(IPropertiesProvider& provider, const pugi::xml_node& propertiesNode);
	};

} // namespace Engine::Serialization
