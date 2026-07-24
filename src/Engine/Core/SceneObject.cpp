#include "Engine/Core/SceneObject.h"

#include "Engine/Core/SceneNodeClone.h"

SceneObject::SceneObject(std::shared_ptr<SceneNode> root) : _root(std::move(root)) {}

const std::shared_ptr<SceneNode>& SceneObject::GetNode() const {
	return _root;
}

SceneObject::operator bool() const {
	return static_cast<bool>(_root);
}

std::shared_ptr<SceneNode> SceneObject::InstantiateOn(const std::shared_ptr<SceneNode>& parent) const {
	return Engine::InstantiateSceneNodeOn(_root, parent);
}
