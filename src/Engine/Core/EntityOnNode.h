#pragma once

#include "Engine/Core/IPropertiesProvider.h"

#include <memory>

class SceneNode;

class EntityOnNode : public Engine::IPropertiesProvider
{
public:
	~EntityOnNode() override = default;

	void AttachTo(const std::shared_ptr<SceneNode>& node);

	std::shared_ptr<SceneNode> GetNode() const;

protected:
	std::weak_ptr<SceneNode> _node;
};

inline void EntityOnNode::AttachTo(const std::shared_ptr<SceneNode>& node) {
	_node = node;
}

inline std::shared_ptr<SceneNode> EntityOnNode::GetNode() const {
	return _node.lock();
}
