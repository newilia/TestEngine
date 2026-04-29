#include "EntityOnNode.h"

#include "SceneNode.h"

void EntityOnNode::AttachTo(const std::shared_ptr<SceneNode>& node) {
	_node = node;
}

std::shared_ptr<SceneNode> EntityOnNode::GetNode() const {
	return _node.lock();
}
