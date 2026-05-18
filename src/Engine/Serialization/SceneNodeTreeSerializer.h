#pragma once

#include "Engine/Serialization/SerializationResult.h"

#include <pugixml.hpp>

class SceneNode;

namespace Engine::Serialization {

	class SceneNodeTreeSerializer
	{
	public:
		static void SaveNode(const SceneNode& sceneNode, pugi::xml_node xmlParent, SerializationResult& result);
		static void LoadNode(const pugi::xml_node& xmlNode, SceneNode& targetNode, SerializationResult& result);
	};

} // namespace Engine::Serialization
