#pragma once

#include "Engine/Editor/SceneCloneUtils.h"

#include <memory>
#include <variant>

class SceneNode;

namespace Engine {

	class SceneClipboard
	{
	public:
		bool CopyNode(const std::shared_ptr<SceneNode>& node);
		bool CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);

		bool HasNode() const;
		bool HasEntity() const;
		EntitySlot GetEntitySlot() const;

		std::shared_ptr<SceneNode> InstantiateNode() const;
		std::shared_ptr<EntityOnNode> InstantiateEntity() const;

	private:
		struct NodePayload
		{
			std::shared_ptr<SceneNode> prototype;
		};

		struct EntityPayload
		{
			EntitySlot slot = EntitySlot::Behaviour;
			std::shared_ptr<EntityOnNode> prototype;
		};

		std::variant<std::monostate, NodePayload, EntityPayload> _payload;
	};

} // namespace Engine
