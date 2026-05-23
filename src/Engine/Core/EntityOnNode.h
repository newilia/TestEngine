#pragma once

#include "Engine/Core/EntityId.h"
#include "Engine/Core/IPropertiesProvider.h"

#include <memory>

class SceneNode;

class EntityOnNode : public Engine::IPropertiesProvider
{
public:
	virtual ~EntityOnNode() = default;

	void AttachTo(const std::shared_ptr<SceneNode>& node);
	std::shared_ptr<SceneNode> GetNode() const;

	Engine::EntityId GetEntityId() const;
	void SetEntityId(Engine::EntityId id);

protected:
	std::weak_ptr<SceneNode> _node;
	Engine::EntityId _entityId = Engine::kInvalidEntityId;
};
