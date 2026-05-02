#include "Engine/Core/Transform.h"

#include "Engine/Core/PropertyTree.h"
#include "Engine/Core/SceneNode.h"

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
		    return getPosition();
	    },
	    [this](sf::Vector2f v) {
		    setPosition(v);
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
		    return getScale();
	    },
	    [this](sf::Vector2f v) {
		    setScale(v);
	    },
	    {});
	b.addDouble(
	    "rotation_deg", "Rotation (deg)",
	    [this] {
		    return static_cast<double>(getRotation().asDegrees());
	    },
	    [this](double deg) {
		    setRotation(sf::degrees(static_cast<float>(deg)));
	    },
	    {});

	b.pop();
}
