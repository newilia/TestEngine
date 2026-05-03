#include "Engine/Core/Transform.h"

#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"

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

void Transform::BuildPropertyTree(Engine::PropertyBuilder& b) {
	b.pushObject("transform", "Transform");

	static const Engine::PropertyMeta globalReadOnly = [] {
		Engine::PropertyMeta m;
		m.readOnly = true;
		return m;
	}();

	b.addVec2f(
	    "position", "Local position",
	    [this] {
		    return getLocalPosition();
	    },
	    [this](sf::Vector2f v) {
		    setLocalPosition(v);
	    },
	    {});
	b.addVec2f(
	    "global_position", "Global position",
	    [this] {
		    auto n = GetNode();
		    return n ? n->GetPosGlobal() : sf::Vector2f{};
	    },
	    [](sf::Vector2f) {
	    },
	    globalReadOnly);
	b.addVec2f(
	    "scale", "Scale",
	    [this] {
		    return getLocalScale();
	    },
	    [this](sf::Vector2f v) {
		    setLocalScale(v);
	    },
	    {});
	b.addDouble(
	    "rotation_deg", "Rotation (deg)",
	    [this] {
		    return static_cast<double>(getLocalRotation().asDegrees());
	    },
	    [this](double deg) {
		    setLocalRotation(sf::degrees(static_cast<float>(deg)));
	    },
	    {});

	b.pop();
}
