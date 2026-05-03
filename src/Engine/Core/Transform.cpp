#include "Engine/Core/Transform.h"

#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"
#include "Transform.generated.hpp"

void Transform::NotifyTransformChanged() {
	if (auto n = GetNode()) {
		n->MarkWorldTransformSubtreeDirty();
	}
}

sf::Transform Transform::GetTransform() const {
	return _transformable.getTransform();
}

sf::Vector2f Transform::GetPosition() const {
	return _transformable.getPosition();
}

void Transform::SetPosition(sf::Vector2f v) {
	_transformable.setPosition(v);
	NotifyTransformChanged();
}

sf::Vector2f Transform::GetScale() const {
	return _transformable.getScale();
}

void Transform::SetScale(sf::Vector2f v) {
	_transformable.setScale(v);
	NotifyTransformChanged();
}

sf::Angle Transform::GetRotation() const {
	return _transformable.getRotation();
}

void Transform::SetRotation(sf::Angle angle) {
	_transformable.setRotation(angle);
	NotifyTransformChanged();
}

sf::Vector2f Transform::GetOrigin() const {
	return _transformable.getOrigin();
}

void Transform::SetOrigin(sf::Vector2f v) {
	_transformable.setOrigin(v);
	NotifyTransformChanged();
}
