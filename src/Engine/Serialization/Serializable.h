#pragma once

#include "Engine/Serialization/SerializationBuilder.h"

namespace Engine::Serialization {

	struct Serializable
	{
		virtual ~Serializable() = default;
		virtual void Serialize(SerializationBuilder& builder, SerializationResult& result) = 0;
	};

} // namespace Engine::Serialization
