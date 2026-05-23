#include "Engine/Core/SceneObject.h"

#include "Engine/Core/SceneNode.h"

SceneObject::SceneObject(std::shared_ptr<SceneNode> root) : _root(std::move(root)) {}

const std::shared_ptr<SceneNode>& SceneObject::GetNode() const {
	return _root;
}

SceneObject::operator bool() const {
	return static_cast<bool>(_root);
}
