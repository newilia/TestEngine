#include "EntityOnNode.h"

#include "SceneNode.h"

void EntityOnNode::AttachTo(const std::shared_ptr<SceneNode>& node) {
	_node = node;
}

std::shared_ptr<SceneNode> EntityOnNode::GetNode() const {
	return _node.lock();
}

Engine::SceneObjectId EntityOnNode::GetSceneObjectId() const {
	return _sceneObjectId;
}

void EntityOnNode::SetSceneObjectId(Engine::SceneObjectId id) {
	_sceneObjectId = id;
}
