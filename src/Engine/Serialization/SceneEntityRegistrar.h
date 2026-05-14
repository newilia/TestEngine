#pragma once

#include "Engine/Serialization/SceneEntityRegistry.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace Engine::Serialization {

	namespace SceneEntityRegistrationDetail {
		void AppendSceneEntityRegistration(SceneEntityRegistration registration);
	}

	template <typename TEntity>
	void RegisterSceneEntity(std::string_view typeId, SceneEntityKind kind) {
		SceneEntityRegistration registration;
		registration.typeId = std::string(typeId);
		registration.kind = kind;
		registration.createInstance = []() -> std::shared_ptr<EntityOnNode> {
			return std::make_shared<TEntity>();
		};
		registration.isType = [](const std::shared_ptr<EntityOnNode>& entity) {
			return std::dynamic_pointer_cast<TEntity>(entity) != nullptr;
		};
		SceneEntityRegistrationDetail::AppendSceneEntityRegistration(std::move(registration));
	}

} // namespace Engine::Serialization
