#pragma once

#include "Engine/Core/EntitySlot.h"
#include "Engine/Core/SceneNodeClone.h"

#include <memory>
#include <variant>

class SceneNode;
class Transform;

namespace Engine {

	class SceneClipboard
	{
	public:
		bool CopyNode(const std::shared_ptr<SceneNode>& node);
		bool CopyEntity(const std::shared_ptr<EntityOnNode>& entity, EntitySlot slot);
		bool CopyNodeTransform(const std::shared_ptr<SceneNode>& node);

		bool HasNode() const;
		bool HasEntity() const;
		EntitySlot GetEntitySlot() const;

		std::shared_ptr<SceneNode> InstantiateNode() const;
		std::shared_ptr<EntityOnNode> InstantiateEntity() const;
		std::shared_ptr<Transform> InstantiateTransform() const;

	private:
		struct NodePayload
		{
			std::shared_ptr<SceneNode> prototype;
		};

		struct EntityPayload
		{
			EntitySlot slot = EntitySlot::Behaviour;
			std::shared_ptr<EntityOnNode> prototype;
			std::shared_ptr<Transform> transformPrototype;
		};

		std::variant<std::monostate, NodePayload, EntityPayload> _payload;
	};

} // namespace Engine
