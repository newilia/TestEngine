#pragma once

#include "Engine/Core/EntityOnNode.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

/// Per-node spatial transform; held by `SceneNode` as local TRS relative to the parent.
/// Mutations use `setLocal*` only so hierarchy caches stay consistent (no public `sf::Transformable`).
class Transform final : public EntityOnNode
{
	META_CLASS()
	META_PROPERTY_BASE(EntityOnNode)

public:
	sf::Transform getTransform() const;

	/// @getter
	sf::Vector2f getLocalPosition() const;
	/// @setter
	void setLocalPosition(sf::Vector2f v);

	/// @getter
	sf::Vector2f getLocalScale() const;
	/// @setter
	void setLocalScale(sf::Vector2f v);

	/// @getter
	sf::Angle getLocalRotation() const;
	/// @setter
	void setLocalRotation(sf::Angle angle);

	/// @getter
	sf::Vector2f getLocalOrigin() const;
	/// @setter
	void setLocalOrigin(sf::Vector2f v);

private:
	void notifyTransformChanged();

	sf::Transformable _transformable;
};
