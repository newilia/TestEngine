#pragma once

#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/SceneObjectId.h"

#include <memory>

class SceneNode;

class EntityOnNode : public Engine::IPropertiesProvider
{
public:
	virtual ~EntityOnNode() = default;

	void AttachTo(const std::shared_ptr<SceneNode>& node);
	std::shared_ptr<SceneNode> GetNode() const;

	Engine::SceneObjectId GetSceneObjectId() const;
	void SetSceneObjectId(Engine::SceneObjectId id);

protected:
	std::weak_ptr<SceneNode> _node;
	Engine::SceneObjectId _sceneObjectId = Engine::kInvalidSceneObjectId;
};
