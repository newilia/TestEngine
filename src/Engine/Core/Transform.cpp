#include "Engine/Core/Transform.h"

#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Transform.generated.hpp"

void Transform::notifyTransformChanged() {
	if (auto n = GetNode()) {
		n->MarkWorldTransformSubtreeDirty();
	}
}

sf::Transform Transform::getTransform() const {
	return _transformable.getTransform();
}

sf::Vector2f Transform::getLocalPosition() const {
	return _transformable.getPosition();
}

void Transform::setLocalPosition(sf::Vector2f v) {
	_transformable.setPosition(v);
	notifyTransformChanged();
}

sf::Vector2f Transform::getLocalScale() const {
	return _transformable.getScale();
}

void Transform::setLocalScale(sf::Vector2f v) {
	_transformable.setScale(v);
	notifyTransformChanged();
}

sf::Angle Transform::getLocalRotation() const {
	return _transformable.getRotation();
}

void Transform::setLocalRotation(sf::Angle angle) {
	_transformable.setRotation(angle);
	notifyTransformChanged();
}

sf::Vector2f Transform::getLocalOrigin() const {
	return _transformable.getOrigin();
}

void Transform::setLocalOrigin(sf::Vector2f v) {
	_transformable.setOrigin(v);
	notifyTransformChanged();
}
