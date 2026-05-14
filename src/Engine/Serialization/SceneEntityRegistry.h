#pragma once

#include "Engine/Core/EntityOnNode.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Engine::Serialization {

	enum class SceneEntityKind
	{
		Transform,
		Visual,
		SortingStrategy,
		Behaviour
	};

	struct SceneEntityRegistration
	{
		std::string typeId;
		SceneEntityKind kind = SceneEntityKind::Behaviour;
		std::function<std::shared_ptr<EntityOnNode>()> createInstance;
		std::function<bool(const std::shared_ptr<EntityOnNode>&)> isType;
	};

	class SceneEntityRegistry
	{
	public:
		static const SceneEntityRegistry& GetInstance();

		const std::vector<SceneEntityRegistration>& GetAll() const;

		std::shared_ptr<EntityOnNode> CreateByTypeId(std::string_view typeId) const;
		std::optional<SceneEntityKind> GetKindByTypeId(std::string_view typeId) const;
		std::string GetTypeIdForEntity(const std::shared_ptr<EntityOnNode>& entity) const;

	private:
		SceneEntityRegistry();

		std::vector<SceneEntityRegistration> _registrations;
	};

} // namespace Engine::Serialization
